#include "DeferredRenderer.h"
#include "ConstantBuffers.h"
#include "VRAMInfo.h"
#include "ModelBinaryLoader.h"
#include "SkyDome.h"
#include "WrapperFactory.h"
#include "Utilities/MemoryUtil.h"
#include <algorithm>	// std::sort
#include <iterator>     // std::back_inserter

using std::vector;

DeferredRenderer::DeferredRenderer()
		
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	m_SpotLights = nullptr;
	m_PointLights = nullptr;
	m_DirectionalLights = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		m_RenderTargets[i] = nullptr;
	}

	m_DiffuseSRV = nullptr;
	m_NormalSRV = nullptr;
	m_LightSRV = nullptr;
	m_wPositionSRV = nullptr;

	m_Sampler = nullptr;
	m_PointShader = nullptr;
	m_SpotShader = nullptr;
	m_DirectionalShader = nullptr;

	m_PointModelBuffer = nullptr;
	m_SpotModelBuffer = nullptr;
	m_DirectionalModelBuffer = nullptr;

	m_ConstantBuffer = nullptr;
	m_ObjectConstantBuffer = nullptr;
	m_AnimatedObjectConstantBuffer = nullptr;
	m_WorldInstanceData = nullptr;
	m_InstancedGeometryShader = nullptr;

	m_AllLightBuffer = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;
	m_CameraPosition = nullptr;

	m_RasterState = nullptr;
	m_DepthState = nullptr;
	m_BlendState = nullptr;
	m_BlendState2 = nullptr;

	m_SkyDomeBuffer = nullptr;
	m_SkyDomeShader = nullptr;
	m_SkyDomeSRV = nullptr;
	m_SkyDomeDepthStencilState = nullptr;
	m_SkyDomeRasterizerState = nullptr;
	m_SkyDomeSampler = nullptr;
}

DeferredRenderer::~DeferredRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	m_SpotLights = nullptr;
	m_PointLights = nullptr;
	m_DirectionalLights = nullptr;

	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;
	m_CameraPosition = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		SAFE_RELEASE(m_RenderTargets[i]);
	}

	SAFE_RELEASE(m_DiffuseSRV);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_LightSRV);
	SAFE_RELEASE(m_wPositionSRV);

	SAFE_RELEASE(m_Sampler);
	SAFE_RELEASE(m_BlendState);
	SAFE_RELEASE(m_BlendState2);

	SAFE_RELEASE(m_RasterState);
	SAFE_RELEASE(m_DepthState);

	SAFE_DELETE(m_PointShader);
	SAFE_DELETE(m_SpotShader);
	SAFE_DELETE(m_DirectionalShader);

	SAFE_DELETE(m_PointModelBuffer);
	SAFE_DELETE(m_SpotModelBuffer);
	SAFE_DELETE(m_DirectionalModelBuffer);

	SAFE_DELETE(m_ConstantBuffer);
	SAFE_DELETE(m_ObjectConstantBuffer);
	SAFE_DELETE(m_AllLightBuffer);

	SAFE_DELETE(m_AnimatedObjectConstantBuffer);
	SAFE_DELETE(m_WorldInstanceData);
	SAFE_DELETE(m_InstancedGeometryShader);	

	SAFE_DELETE(m_SkyDomeBuffer);
	SAFE_DELETE(m_SkyDomeShader);
	SAFE_RELEASE(m_SkyDomeSRV);
	SAFE_RELEASE(m_SkyDomeDepthStencilState);
	SAFE_RELEASE(m_SkyDomeRasterizerState);
	SAFE_RELEASE(m_SkyDomeSampler);
}

void DeferredRenderer::initialize(ID3D11Device* p_Device, ID3D11DeviceContext* p_DeviceContext,
	ID3D11DepthStencilView *p_DepthStencilView, unsigned int p_screenWidth, unsigned int p_screenHeight,
	DirectX::XMFLOAT3 *p_CameraPosition, DirectX::XMFLOAT4X4 *p_ViewMatrix,	DirectX::XMFLOAT4X4 *p_ProjectionMatrix,
	std::vector<Light> *p_SpotLights, std::vector<Light> *p_PointLights, std::vector<Light> *p_DirectionalLights,
	unsigned int p_MaxLightsPerLightInstance)
{
	m_Device			= p_Device;
	m_DeviceContext		= p_DeviceContext;
	m_DepthStencilView	= p_DepthStencilView;

	m_CameraPosition	= p_CameraPosition;
	m_ViewMatrix		= p_ViewMatrix;
	m_ProjectionMatrix	= p_ProjectionMatrix;

	m_SpotLights = p_SpotLights;
	m_PointLights = p_PointLights;
	m_DirectionalLights = p_DirectionalLights;
	m_RenderSkyDome = false;
	m_MaxLightsPerLightInstance = p_MaxLightsPerLightInstance;

	//Create render targets with the size of screen width and screen height
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.Width				= p_screenWidth;
	desc.Height				= p_screenHeight;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count	= 1;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	createRenderTargets(desc);

	createShaderResourceViews(desc);

	createLightShaders();

	loadLightModels();

	// Create sampler state and blend state for Alpha rendering.
	createSamplerState();

	createBlendStates();
	createLightStates();
	createBuffers();
}

void DeferredRenderer::renderDeferred()
{
	// Clear render targets.
	clearRenderTargets();

	//// Update constant buffer
	updateConstantBuffer();

	// Render
	if(m_Objects.size() > 0)
	{
		renderGeometry();
		renderLighting();
	}
	m_RenderSkyDome = false;
}

void DeferredRenderer::renderGeometry()
{
	unsigned int nrRT = 3;

	// Set the render targets.
	m_DeviceContext->OMSetRenderTargets(nrRT, m_RenderTargets, m_DepthStencilView);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// The textures will be needed to be grabbed from the model later.

	std::sort(m_Objects.begin(),m_Objects.end(), [] (Renderable &a,Renderable &b)
	{ 
		return a.model->vertexBuffer > b.model->vertexBuffer;
	});

	std::vector<std::vector<Renderable>> ttemp;
	std::vector<Renderable> ttani;

	for(unsigned int i = 0; i < m_Objects.size(); i++)
	{
		if(m_Objects.at(i).model->isAnimated)
			ttani.push_back(std::move(m_Objects.at(i)));
		else
		{
			std::vector<Renderable> l;
			int current = i;
			int nr = 0;
			for(unsigned int j = i + 1; j < m_Objects.size();j++)
				if(m_Objects.at(current).model->vertexBuffer == m_Objects.at(j).model->vertexBuffer)
					nr++;
				else
					break;

			i += nr;

			if(nr >= 1)
			{
				std::move(m_Objects.begin() + current, m_Objects.begin() + current + nr + 1, std::back_inserter(l));
				ttemp.push_back(l);
			}
			else
				ttani.push_back(std::move(m_Objects.at(current)));
		}
	}
	m_Objects.clear();

	m_ConstantBuffer->setBuffer(1);
	m_DeviceContext->PSSetSamplers(0,1,&m_Sampler);
	updateConstantBuffer();
	for( auto &animation : ttani )
		renderObject(animation);

	for( auto &k : ttemp)
	{
		UINT Offsets[2] = {0,0};
		ID3D11Buffer * buffers[] = {k.front().model->vertexBuffer->getBufferPointer(), m_WorldInstanceData->getBufferPointer()};
		UINT Stride[2] = {60, sizeof(DirectX::XMFLOAT4X4)};


		ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};

		// Set shader.
		m_InstancedGeometryShader->setShader();
		float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
		m_InstancedGeometryShader->setBlendState(m_BlendState2, data);
		m_DeviceContext->IASetVertexBuffers(0,2,buffers,Stride, Offsets);

		for(unsigned int u = 0; u < k.front().model->numOfMaterials;u++)
		{
			ID3D11ShaderResourceView *srvs[] =  {	k.front().model->diffuseTexture[u].second, 
													k.front().model->normalTexture[u].second, 
													k.front().model->specularTexture[u].second 
												};
			m_DeviceContext->PSSetShaderResources(0, 3, srvs);
			D3D11_MAPPED_SUBRESOURCE ms;
			for(unsigned int i = 0; i < k.size(); i += m_MaxLightsPerLightInstance)
			{
				int nrToCpy = (k.size() - i >= m_MaxLightsPerLightInstance) ? m_MaxLightsPerLightInstance : k.size() - i ;
				std::vector<DirectX::XMFLOAT4X4> tWorld;
				for(int j = 0; j < nrToCpy; j++)
					tWorld.push_back(k.at(i+j).world);

				m_DeviceContext->Map(m_WorldInstanceData->getBufferPointer(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
				memcpy(ms.pData, tWorld.data(), sizeof(DirectX::XMFLOAT4X4) * tWorld.size());
				m_DeviceContext->Unmap(m_WorldInstanceData->getBufferPointer(), NULL);

				m_DeviceContext->DrawInstanced(k.front().model->drawInterval.at(u).second, tWorld.size(),
					k.front().model->drawInterval.at(u).first,0);
			}
			m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
		}

		for(unsigned int i = 0; i < 2; i++)
			m_DeviceContext->IASetVertexBuffers(i,0,0,0, 0);
		m_InstancedGeometryShader->setBlendState(0, data);
		m_InstancedGeometryShader->unSetShader();
	}
	/*for( auto &o : m_Objects)
	{
		renderObject(o);
	}*/	
	m_DeviceContext->PSSetSamplers(0,0,0);
	m_ConstantBuffer->unsetBuffer(1);

	// Unset render targets.
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::renderLighting()
{
	unsigned int activeRenderTarget	= 3;
	unsigned int nrRT = 1;

	// Store previous States to be set when we exit the method.
	ID3D11RasterizerState *previousRasterState;
	ID3D11DepthStencilState *previousDepthState;
	m_DeviceContext->RSGetState(&previousRasterState);
	m_DeviceContext->OMGetDepthStencilState(&previousDepthState,0);

	// Collect the shader resources in an array and create a clear array.
	ID3D11ShaderResourceView *srvs[] = {m_wPositionSRV, m_NormalSRV, m_DiffuseSRV};
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};

	// Set texture sampler.
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	UINT sampleMask = 0xffffffff;
	m_DeviceContext->PSSetShaderResources(0, 3, srvs);

	////Select the third render target[3]
	m_DeviceContext->OMSetRenderTargets(nrRT, &m_RenderTargets[activeRenderTarget], m_DepthStencilView); 
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_DeviceContext->RSSetState(m_RasterState);
	m_DeviceContext->OMSetDepthStencilState(m_DepthState,0);
	m_DeviceContext->OMSetBlendState(m_BlendState, blendFactor, sampleMask);

	//Set constant data
	m_ConstantBuffer->setBuffer(0);

	//		Render PointLights
	renderLight(m_PointShader, m_PointModelBuffer, m_PointLights);
	//		Render SpotLights
	renderLight(m_SpotShader, m_SpotModelBuffer, m_SpotLights);
	//		Render DirectionalLights
	m_DeviceContext->OMSetRenderTargets(nrRT, &m_RenderTargets[activeRenderTarget],0);
	renderLight(m_DirectionalShader, m_DirectionalModelBuffer, m_DirectionalLights);

	renderSkyDomeImpl();

	m_ConstantBuffer->unsetBuffer(0);
	m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
	m_DeviceContext->RSSetState(previousRasterState);
	m_DeviceContext->OMSetDepthStencilState(previousDepthState,0);
	m_DeviceContext->OMSetBlendState(0, blendFactor, sampleMask);
	SAFE_RELEASE(previousRasterState);
	SAFE_RELEASE(previousDepthState);
}

void DeferredRenderer::renderSkyDomeImpl()
{
	if(m_RenderSkyDome)
	{
		////Select the third render target[3]
		m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargets[3], m_DepthStencilView); 
		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_DeviceContext->RSSetState(m_SkyDomeRasterizerState);
		m_DeviceContext->OMSetDepthStencilState(m_SkyDomeDepthStencilState,0);
		m_DeviceContext->PSSetSamplers(0,1,&m_SkyDomeSampler);
		m_DeviceContext->PSSetShaderResources(0,1,&m_SkyDomeSRV);
		//Set constant data
		m_ConstantBuffer->setBuffer(0);

		m_SkyDomeShader->setShader();
		m_SkyDomeBuffer->setBuffer(0);

		m_DeviceContext->Draw(m_SkyDomeBuffer->getNumOfElements(),0);

		m_SkyDomeBuffer->unsetBuffer(0);
		m_SkyDomeShader->unSetShader();
		m_ConstantBuffer->unsetBuffer(0);
		m_DeviceContext->PSSetSamplers(0,0,0);
	}
}

void DeferredRenderer::addRenderable(Renderable p_renderable)
{
	m_Objects.push_back(p_renderable);
}
void DeferredRenderer::createSkyDome(ID3D11ShaderResourceView* p_Texture, float p_Radius)
{
	ID3D11Resource *resource;
	ID3D11Texture2D *texture;
	D3D11_TEXTURE2D_DESC textureDesc;

	p_Texture->GetResource(&resource);
	resource->QueryInterface(&texture);
	texture->GetDesc(&textureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = textureDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    viewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
    viewDesc.TextureCube.MostDetailedMip = 0;

	m_Device->CreateShaderResourceView(texture, &viewDesc, &m_SkyDomeSRV);
	SAFE_RELEASE(texture);
	SAFE_RELEASE(resource);

	SkyDome d;
	d.init(p_Radius);
	std::vector<DirectX::XMFLOAT3> tt = d.getVertices();
	Buffer::Description cbdesc;
	cbdesc.initData = tt.data();
	cbdesc.numOfElements = tt.size();
	cbdesc.sizeOfElement = sizeof(DirectX::XMFLOAT3);
	cbdesc.type = Buffer::Type::VERTEX_BUFFER;
	cbdesc.usage = Buffer::Usage::USAGE_IMMUTABLE;
	m_SkyDomeBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory( &dsdesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	dsdesc.DepthEnable = true;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_Device->CreateDepthStencilState(&dsdesc, &m_SkyDomeDepthStencilState);

	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory( &rdesc, sizeof( D3D11_RASTERIZER_DESC ) );
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_NONE;
	m_Device->CreateRasterizerState(&rdesc,&m_SkyDomeRasterizerState);

	m_SkyDomeShader = WrapperFactory::getInstance()->createShader(L"../../Graphics/Source/DeferredShaders/SkyDome.hlsl",
		"VS,PS","5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER);

}
void DeferredRenderer::renderSkyDome()
{
	m_RenderSkyDome = true;
}
ID3D11ShaderResourceView* DeferredRenderer::getRT(int i)
{
	switch(i)
	{
		case 0: return m_DiffuseSRV;
		case 1: return m_NormalSRV;
		case 2: return m_wPositionSRV;
		case 3: return m_LightSRV;
		default: return nullptr;
	}
}

void DeferredRenderer::updateConstantBuffer()
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = *m_CameraPosition;
	m_DeviceContext->UpdateSubresource(m_ConstantBuffer->getBufferPointer(), NULL,NULL, &cb,NULL,NULL);
}

HRESULT DeferredRenderer::createRenderTargets(D3D11_TEXTURE2D_DESC &desc)
{
	// Create the render target texture
	HRESULT result = S_FALSE;

	//Create the render targets
	ID3D11Texture2D *srvt0, *srvt1, *srvt2, *srvt3;
	srvt0 = srvt1 = srvt2 = srvt3 = nullptr;

	result = m_Device->CreateTexture2D(&desc, nullptr, &srvt0);
	if(FAILED(result))
		return result;
	result = m_Device->CreateTexture2D(&desc, nullptr, &srvt1);
	if(FAILED(result))
		return result;
	result = m_Device->CreateTexture2D(&desc, nullptr, &srvt2);
	if(FAILED(result))
		return result;
	result = m_Device->CreateTexture2D(&desc, nullptr, &srvt3);
	if(FAILED(result))
		return result;

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = desc.Format;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;

	result = m_Device->CreateRenderTargetView(srvt0, &rtDesc, &m_RenderTargets[0]);
	if(FAILED(result))
		return result;
	result = m_Device->CreateRenderTargetView(srvt1, &rtDesc, &m_RenderTargets[1]);
	if(FAILED(result))
		return result;
	result = m_Device->CreateRenderTargetView(srvt2, &rtDesc, &m_RenderTargets[2]);
	if(FAILED(result))
		return result;
	result = m_Device->CreateRenderTargetView(srvt3, &rtDesc, &m_RenderTargets[3]);
	if(FAILED(result))
		return result;

	SAFE_RELEASE(srvt0);
	SAFE_RELEASE(srvt1);
	SAFE_RELEASE(srvt2);
	SAFE_RELEASE(srvt3);
	// Done with the render targets.

	unsigned int size = 4 * VRAMInfo::getInstance()->calculateFormatUsage(desc.Format, desc.Width, desc.Height);
	VRAMInfo::getInstance()->updateUsage(size);

	return result;
}

HRESULT DeferredRenderer::createShaderResourceViews( D3D11_TEXTURE2D_DESC &desc )
{
	HRESULT result = E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC dssrvdesc;
	dssrvdesc.Format = dssrvdesc.Format = desc.Format;
	dssrvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dssrvdesc.Texture2D.MipLevels = 1;
	dssrvdesc.Texture2D.MostDetailedMip = 0;

	ID3D11Resource* tt;

	// Make the diffuse texture from the render target.	
	m_RenderTargets[0]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_DiffuseSRV);
	SAFE_RELEASE(tt);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the normal texture from the render target.
	m_RenderTargets[1]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_NormalSRV);
	SAFE_RELEASE(tt);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the world position texture from the render target.
	m_RenderTargets[2]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_wPositionSRV);
	SAFE_RELEASE(tt);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the final texture from the render target.
	m_RenderTargets[3]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_LightSRV);
	SAFE_RELEASE(tt);
	tt = nullptr;
	if(FAILED(result))
		return result;

	return result;
}

void DeferredRenderer::createBuffers()
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = *m_CameraPosition;

	Buffer::Description cbdesc;
	cbdesc.initData = &cb;
	cbdesc.numOfElements = 1;
	cbdesc.sizeOfElement = sizeof(cBuffer);
	cbdesc.type = Buffer::Type::CONSTANT_BUFFER_ALL;
	cbdesc.usage = Buffer::Usage::DEFAULT;

	m_ConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cBuffer));

	cbdesc.initData = nullptr;
	cbdesc.sizeOfElement = sizeof(cObjectBuffer);
	m_ObjectConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cObjectBuffer));

	cbdesc.sizeOfElement = sizeof(cAnimatedObjectBuffer);
	m_AnimatedObjectConstantBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	VRAMInfo::getInstance()->updateUsage(sizeof(cAnimatedObjectBuffer));	

	Buffer::Description adesc;
	adesc.initData = nullptr;
	adesc.numOfElements = m_MaxLightsPerLightInstance;
	adesc.sizeOfElement = sizeof(Light);
	adesc.type = Buffer::Type::VERTEX_BUFFER;
	adesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
	m_AllLightBuffer = WrapperFactory::getInstance()->createBuffer(adesc);

	VRAMInfo::getInstance()->updateUsage(sizeof(Light) * m_MaxLightsPerLightInstance);

	Buffer::Description instanceWorldDesc;
	instanceWorldDesc.initData = nullptr;
	instanceWorldDesc.numOfElements = m_MaxLightsPerLightInstance;
	instanceWorldDesc.sizeOfElement = sizeof(DirectX::XMFLOAT4X4);
	instanceWorldDesc.type = Buffer::Type::VERTEX_BUFFER;
	instanceWorldDesc.usage = Buffer::Usage::CPU_WRITE_DISCARD;
	m_WorldInstanceData = WrapperFactory::getInstance()->createBuffer(instanceWorldDesc);

	VRAMInfo::getInstance()->updateUsage(sizeof(DirectX::XMFLOAT4X4) * m_MaxLightsPerLightInstance);
}

void DeferredRenderer::clearRenderTargets()
{
	float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_DeviceContext->ClearRenderTargetView(m_RenderTargets[0], color);

	color[0] = color[1] = color[2] = 0.5f;
	m_DeviceContext->ClearRenderTargetView(m_RenderTargets[1], color);

	color[0] = color[1] = color[2] = 1.0f;
	m_DeviceContext->ClearRenderTargetView(m_RenderTargets[2], color);

	color[0] = color[1] = color[2] = 0.0f;
	m_DeviceContext->ClearRenderTargetView(m_RenderTargets[3], color);
}

void DeferredRenderer::createSamplerState()
{
	// Create texture sampler.
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter			= D3D11_FILTER_ANISOTROPIC;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc   = D3D11_COMPARISON_NEVER;
	sd.MinLOD			= 0;
	sd.MaxLOD			= D3D11_FLOAT32_MAX;

	m_Device->CreateSamplerState( &sd, &m_Sampler );
	// Create texture sampler.
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.Filter			= D3D11_FILTER_ANISOTROPIC;

	m_Device->CreateSamplerState( &sd, &m_SkyDomeSampler );
}

void DeferredRenderer::createBlendStates()
{
	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend =  D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_Device->CreateBlendState(&bd, &m_BlendState);

	for(int i = 0; i < 4; i++)
	{
		bd.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlend =  D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	}

	m_Device->CreateBlendState(&bd, &m_BlendState2);
}

void DeferredRenderer::createLightShaders()
{
	ShaderInputElementDescription shaderDesc[] = 
	{
		{"POSITION",	0, Format::R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	  0},
		{"LPOSITION",	0, Format::R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA,  1},
		{"COLOR",		0, Format::R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"DIRECTION",	0, Format::R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"ANGLE",		0, Format::R32G32_FLOAT,	1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"RANGE",		0, Format::R32_FLOAT,		1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	m_SpotShader = WrapperFactory::getInstance()->createShader(L"../../Graphics/Source/DeferredShaders/LightPassSpotLight.hlsl",
		"SpotLightVS,SpotLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 6);

	m_PointShader = WrapperFactory::getInstance()->createShader(L"../../Graphics/Source/DeferredShaders/LightPassPointLight.hlsl",
		"PointLightVS,PointLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 6);

	m_DirectionalShader = WrapperFactory::getInstance()->createShader(L"../../Graphics/Source/DeferredShaders/LightPassDirectionalLight.hlsl",
		"DirectionalLightVS,DirectionalLightPS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, shaderDesc, 6);

	ShaderInputElementDescription instanceshaderDesc[] = 
	{
		{"POSITION",0, Format::R32G32B32A32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	0, Format::R32G32B32_FLOAT, 0, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COORD",	0, Format::R32G32_FLOAT, 0, 28,		D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",	0, Format::R32G32B32_FLOAT, 0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL",0, Format::R32G32B32_FLOAT, 0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WORLD", 0, Format::R32G32B32A32_FLOAT, 1, 0,	D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 1, Format::R32G32B32A32_FLOAT, 1, 16, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 2, Format::R32G32B32A32_FLOAT, 1, 32, D3D10_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 3, Format::R32G32B32A32_FLOAT, 1, 48, D3D10_INPUT_PER_INSTANCE_DATA, 1},
	};

	m_InstancedGeometryShader = WrapperFactory::getInstance()->createShader(L"../../Graphics/Source/DeferredShaders/GeoInstanceShader.hlsl",
		"VS,PS", "5_0",ShaderType::VERTEX_SHADER | ShaderType::PIXEL_SHADER, instanceshaderDesc, 9);
}

void DeferredRenderer::loadLightModels()
{
	ModelBinaryLoader modelLoader;
	modelLoader.loadBinaryFile("../../Client/Bin/assets/LightModels/SpotLight.btx");
	const std::vector<StaticVertex>& vertices = modelLoader.getStaticVertexBuffer();
	std::vector<DirectX::XMFLOAT3> temp;
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		temp.push_back(DirectX::XMFLOAT3(vertices.at(i).m_Position.x,vertices.at(i).m_Position.y,vertices.at(i).m_Position.z));
	}


	Buffer::Description cbdesc;
	cbdesc.initData = temp.data();
	cbdesc.numOfElements = temp.size();
	cbdesc.sizeOfElement = sizeof(DirectX::XMFLOAT3);
	cbdesc.type = Buffer::Type::VERTEX_BUFFER;
	cbdesc.usage = Buffer::Usage::USAGE_IMMUTABLE;

	m_SpotModelBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	temp.clear();
	modelLoader.loadBinaryFile("../../Client/Bin/assets/LightModels/Sphere2.btx");
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		temp.push_back(DirectX::XMFLOAT3(vertices.at(i).m_Position.x,vertices.at(i).m_Position.y,vertices.at(i).m_Position.z));
	}

	cbdesc.initData = temp.data();
	cbdesc.numOfElements = temp.size();
	m_PointModelBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);
	temp.clear();
	modelLoader.clear();

	//Create a quad
	temp.push_back(DirectX::XMFLOAT3(-1,1,0));
	temp.push_back(DirectX::XMFLOAT3(-1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,1,0));
	temp.push_back(DirectX::XMFLOAT3(-1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,-1,0));
	temp.push_back(DirectX::XMFLOAT3(1,1,0));
	cbdesc.initData = temp.data();
	cbdesc.numOfElements = 6;
	m_DirectionalModelBuffer = WrapperFactory::getInstance()->createBuffer(cbdesc);

	temp.clear();
	temp.shrink_to_fit();
}

void DeferredRenderer::createLightStates()
{
	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory( &dsdesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	dsdesc.DepthEnable = true;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	m_Device->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory( &rdesc, sizeof( D3D11_RASTERIZER_DESC ) );
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_FRONT;
	m_Device->CreateRasterizerState(&rdesc,&m_RasterState);
}

void DeferredRenderer::renderLight(Shader *p_Shader, Buffer* p_ModelBuffer, vector<Light> *p_Lights)
{
	if(p_Lights->size() > 0)
	{
		UINT Offsets[2] = {0,0};
		ID3D11Buffer * buffers[] = {p_ModelBuffer->getBufferPointer(), m_AllLightBuffer->getBufferPointer()};
		UINT Stride[2] = {sizeof(DirectX::XMFLOAT3), sizeof(Light)};

		p_Shader->setShader();
		m_DeviceContext->IASetVertexBuffers(0,2,buffers,Stride, Offsets);
		D3D11_MAPPED_SUBRESOURCE ms;
		for(unsigned int i = 0; i < p_Lights->size(); i += m_MaxLightsPerLightInstance)
		{
			int nrToCpy = (p_Lights->size() - i >= m_MaxLightsPerLightInstance) ? m_MaxLightsPerLightInstance : p_Lights->size() - i ;

			m_DeviceContext->Map(m_AllLightBuffer->getBufferPointer(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, p_Lights->data() + i, sizeof(Light) * nrToCpy);
			m_DeviceContext->Unmap(m_AllLightBuffer->getBufferPointer(), NULL);
			
			m_DeviceContext->DrawInstanced(p_ModelBuffer->getNumOfElements(), p_Lights->size(),0,0);
		}
		
		for(unsigned int i = 0; i < 2; i++)
			m_DeviceContext->IASetVertexBuffers(i,0,0,0, 0);
		p_Shader->unSetShader();
	}
}

void DeferredRenderer::renderObject(Renderable &p_Object)
{
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};
	p_Object.model->vertexBuffer->setBuffer(0);

	if (p_Object.model->isAnimated)
	{
		cAnimatedObjectBuffer temp;
		temp.invTransposeWorld = p_Object.invTransposeWorld;

		const std::vector<DirectX::XMFLOAT4X4>* tempBones = p_Object.finalTransforms;
		for (unsigned int a = 0; a < tempBones->size(); a++)
			temp.boneTransform[a] = (*tempBones)[a];

		m_DeviceContext->UpdateSubresource(m_AnimatedObjectConstantBuffer->getBufferPointer(), NULL,NULL, &temp,NULL,NULL);
		m_AnimatedObjectConstantBuffer->setBuffer(3);
	}

	cObjectBuffer temp;
	temp.world = p_Object.world;
	m_DeviceContext->UpdateSubresource(m_ObjectConstantBuffer->getBufferPointer(), NULL,NULL, &temp,NULL,NULL);
	m_ObjectConstantBuffer->setBuffer(2);

	// Set shader.
	p_Object.model->shader->setShader();
	float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
	p_Object.model->shader->setBlendState(m_BlendState2, data);

	for(unsigned int j = 0; j < p_Object.model->numOfMaterials;j++)
	{
		ID3D11ShaderResourceView *srvs[] =  {	p_Object.model->diffuseTexture[j].second, 
												p_Object.model->normalTexture[j].second, 
												p_Object.model->specularTexture[j].second 
											};
		m_DeviceContext->PSSetShaderResources(0, 3, srvs);

		m_DeviceContext->Draw(p_Object.model->drawInterval.at(j).second, p_Object.model->drawInterval.at(j).first);

		m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	}

	p_Object.model->shader->setBlendState(0, data);
	p_Object.model->shader->unSetShader();
	m_ObjectConstantBuffer->unsetBuffer(2);
	m_AnimatedObjectConstantBuffer->unsetBuffer(3);
	p_Object.model->vertexBuffer->unsetBuffer(0);
}
