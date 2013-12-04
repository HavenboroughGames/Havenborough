#include "DeferredRenderer.h"

DeferredRenderer::DeferredRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		m_RenderTargets[i] = nullptr;
	}

	m_DepthStencilResourceView = nullptr;
	m_DiffuseSRV = nullptr;
	m_NormalSRV = nullptr;
	m_LightSRV = nullptr;
	m_wPositionSRV = nullptr;

	m_Sampler = nullptr;
	m_LightShader = nullptr;
	m_LightBuffer = nullptr;
	m_ConstantBuffer = nullptr;
	m_AllLightBuffer = nullptr;

	m_CameraPosition = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;

	m_speed = 4.0f;
}

DeferredRenderer::~DeferredRenderer(void)
{
	m_Objects.clear();
	m_TransparentObjects.clear();
	m_Lights.clear();

	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		SAFE_RELEASE(m_RenderTargets[i]);
	}
	
	SAFE_RELEASE(m_DepthStencilResourceView);
	SAFE_RELEASE(m_DiffuseSRV);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_LightSRV);
	SAFE_RELEASE(m_wPositionSRV);
	
	SAFE_RELEASE(m_Sampler);
	SAFE_DELETE(m_LightShader);
	SAFE_DELETE(m_LightBuffer);
	SAFE_DELETE(m_ConstantBuffer);
	SAFE_DELETE(m_AllLightBuffer);

	m_CameraPosition = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;
}

void DeferredRenderer::shutdown()
{

}

void DeferredRenderer::initialize(ID3D11Device* p_Device, ID3D11DeviceContext* p_DeviceContext,
								  ID3D11DepthStencilView *p_DepthStencilView,
									unsigned int p_screenWidth, unsigned int p_screenHeight,
					DirectX::XMFLOAT3 *p_CameraPosition, DirectX::XMFLOAT4X4 *p_ViewMatrix, DirectX::XMFLOAT4X4 *p_ProjectionMatrix)
{
	m_Device = p_Device;
	m_DeviceContext = p_DeviceContext;
	m_DepthStencilView = p_DepthStencilView;

	m_CameraPosition = p_CameraPosition;
	m_ViewMatrix = p_ViewMatrix;
	m_ProjectionMatrix = p_ProjectionMatrix;

	//Create render targets with the size of screen width and screen height
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.Width = p_screenWidth;
	desc.Height = p_screenHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	createRenderTargets(desc, p_screenWidth, p_screenHeight);

	createShaderResourceViews(p_DepthStencilView, desc);

	//Create shader for the light pass
	m_LightShader = new Shader();
	m_LightShader->initialize(m_Device,m_DeviceContext, 0);
	m_LightShader->compileAndCreateShader(L"../../Graphics/Source/DummyLightPass.hlsl","VSmain","vs_5_0",VERTEX_SHADER, nullptr);
	m_LightShader->compileAndCreateShader(L"../../Graphics/Source/DummyLightPass.hlsl","PSmain","ps_5_0",PIXEL_SHADER, nullptr);

	//Create texture sampler
	D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc   = D3D11_COMPARISON_NEVER;
    sd.MinLOD			= 0;
    sd.MaxLOD			= D3D11_FLOAT32_MAX;

    m_Device->CreateSamplerState( &sd, &m_Sampler );

	D3D11_RASTERIZER_DESC rdesc;
	rdesc.CullMode = D3D11_CULL_NONE;
	//rdesc.FrontCounterClockwise = true;

	m_Device->CreateRasterizerState(&rdesc,&m_RasterState);
	
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
		bd.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[i].DestBlend =  D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	}
	m_Device->CreateBlendState(&bd, &m_BlendState2);

	// TEMPORARY -----------------------------------------------------------
	// Make the light
	Light testLight(	
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), 
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), 
		DirectX::XMFLOAT2(1.0f, 1.0f),  
		50.0f,
		0
	);
	addLight(testLight);
	
	xx = 10;
	yy = 2;
	zz = 2;
	int minX,minY,minZ,maxX,maxY,maxZ;
	minX = minY = minZ = -30;
	maxX = maxY = maxZ = 30;
	float dx,dy,dz;
	dx = (float)(abs(maxX) + abs(minX))/(xx-1);
	dy = (float)(abs(maxY) + abs(minY))/(yy-1);
	dz = (float)(abs(maxZ) + abs(minZ))/(zz-1);
	
	for(int x= 0; x < xx; x++)
	{
		for(int y = 0; y < yy; y++)
		{
			for(int z = 0; z < zz; z++)
			{
				testLight.lightPos.x = (x * dx) + minX;
				testLight.lightPos.y = (y * dy) + minY;
				testLight.lightPos.z = (z * dz) + minZ;
	
				addLight(testLight);
			}
		}
	}
	BufferDescription cbdesc;
	cbdesc.initData = m_Lights.data();
	cbdesc.numOfElements = xx*yy*zz;
	cbdesc.sizeOfElement = sizeof(Light);
	cbdesc.type = STRUCTURED_BUFFER;
	cbdesc.usage = BUFFER_DEFAULT;
	m_AllLightBuffer = new Buffer();
	m_AllLightBuffer->initializeEx(m_Device, m_DeviceContext, cbdesc, true, false);

	m_TextureLoader = new TextureLoader(m_Device, m_DeviceContext);
	m_Specular = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/diff.jpg");
	m_Diffuse = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/uv alpha.png");
	m_NormalMap = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/Cube1_NRM_RGB.jpg");


	// TEMPORARY ----------------------------------------------------------------
	createConstantBuffer(xx*yy*zz);
}

void DeferredRenderer::renderDeferred()
{
	float dt = 0.016;
	for(int i = 0; i < m_Lights.size();i++)
	{

		if(i%2 == 0)
			m_Lights.at(i).lightPos.y += m_speed * dt;
		else
			m_Lights.at(i).lightPos.y -= m_speed * dt;	
	}
	if((m_Lights.at(0).lightPos.y > 30) ||
		(m_Lights.at(0).lightPos.y < -30))
		m_speed *= -1.0f;


	// Clear render targets.
	clearRenderTargets();


	// Render
	renderGeometry();

	
	// Update constant buffer
	updateConstantBuffer(xx*yy*zz);
	renderLighting();
	renderFinal();
	renderForward();
	
	m_Objects.clear();
	m_TransparentObjects.clear();
	//m_Lights.clear(); For debug this is not active
}

void DeferredRenderer::renderGeometry()
{
	unsigned int nrRT = 3;
	
	// Set the render targets.
	m_DeviceContext->OMSetRenderTargets(nrRT, m_RenderTargets, m_DepthStencilView);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_DeviceContext->RSSetState(m_RasterState);

	m_DeviceContext->PSSetSamplers(0,1,&m_Sampler);
	ID3D11ShaderResourceView *srvs[] = {m_Diffuse,
										m_NormalMap,
										m_Specular};
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};

	m_ConstantBuffer->setBuffer(1);
	m_DeviceContext->PSSetShaderResources(0, 3, srvs);
	for(unsigned int i = 0; i < m_Objects.size();i++)
	//for( int i = m_Objects.size()-1; i >= 0 ;i--)
	{
		updateConstantBuffer(int(i));
		//Send stuff
		// The update of the sub resource has to be done externally.
		m_Objects.at(i).m_Buffer->setBuffer(0);
		//set shader
		m_Objects.at(i).m_Shader->setShader();
		m_Objects.at(i).m_Shader->setBlendState(m_BlendState2);

		m_DeviceContext->Draw(m_Objects.at(i).m_Buffer->getNumOfElements(), 0);

		m_Objects.at(i).m_Buffer->unsetBuffer(0);
		m_Objects.at(i).m_Shader->unSetShader();
	}
	m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	m_ConstantBuffer->unsetBuffer(1);
	// Unset render targets.
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::renderLighting()
{
	unsigned int activeRenderTarget	= 3;
	unsigned int nrRT = 1;
	ID3D11ShaderResourceView* temp;
	temp = m_AllLightBuffer->CreateBufferSRV(m_AllLightBuffer->getBufferPointer());
	// Collect the shader resources in an array and create a clear array.
	ID3D11ShaderResourceView *srvs[] = {m_wPositionSRV,
										m_NormalSRV,
										m_DiffuseSRV,
										temp};
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0,0};

	// Set texture sampler.
	m_DeviceContext->PSSetSamplers(0,1,&m_Sampler);
	m_LightShader->setBlendState(m_BlendState);

	m_DeviceContext->PSSetShaderResources(0, 4, srvs);

	////Select the third render target[3]
	m_DeviceContext->OMSetRenderTargets(nrRT, &m_RenderTargets[activeRenderTarget], 0); 
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_ConstantBuffer->setBuffer(0);
	//for(unsigned int i = 0; i < m_Lights.size();i++)
	//{
		//Send stuff
		//updateLightBuffer(m_Lights.at(i));		
		//m_LightBuffer->setBuffer(1);
		
		//set shader
		m_LightShader->setShader();
		
		m_DeviceContext->Draw(6, 0);
		//unset resources
		//m_LightBuffer->unsetBuffer(1);		
		m_LightShader->unSetShader();
	//}
	m_ConstantBuffer->unsetBuffer(0);
	m_LightShader->setBlendState(0);

	m_DeviceContext->PSSetShaderResources(0, 4, nullsrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::renderFinal()
{

}

void DeferredRenderer::renderForward()
{

}

void DeferredRenderer::addRenderable(Renderable p_renderable, bool p_Transparent)
{
	std::vector<Renderable>* temp;
	if(p_Transparent)
	{
		temp = &m_TransparentObjects;
	}
	else
	{
		temp = &m_Objects;
	}
	temp->push_back(p_renderable);
	temp = nullptr;
}

void DeferredRenderer::addLight(Light p_Light)
{
	m_Lights.push_back(p_Light);
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
void DeferredRenderer::updateConstantBuffer(int nrLights)
{
	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = *m_CameraPosition;
	cb.nrLights = nrLights;
	m_DeviceContext->UpdateSubresource(m_ConstantBuffer->getBufferPointer(), NULL,NULL, &cb,NULL,NULL);
	m_DeviceContext->UpdateSubresource(m_AllLightBuffer->getBufferPointer(), NULL,NULL, m_Lights.data(),NULL,NULL);
}

void DeferredRenderer::updateLightBuffer(Light &p_Light)
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer->getBufferPointer(), NULL,NULL, &p_Light,NULL,NULL);
}

HRESULT DeferredRenderer::createRenderTargets(D3D11_TEXTURE2D_DESC &desc, unsigned int p_screenWidth, unsigned int p_screenHeight )
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

	srvt0 = srvt1 = srvt2 = srvt3 = nullptr;
	//Done with the render targets
	return result;
}

HRESULT DeferredRenderer::createShaderResourceViews( ID3D11DepthStencilView * p_DepthStencilView, D3D11_TEXTURE2D_DESC &desc )
{
	HRESULT result = E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC dssrvdesc;
	dssrvdesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	dssrvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	dssrvdesc.Texture2D.MipLevels = 1;
	dssrvdesc.Texture2D.MostDetailedMip = 0;

	// Make the depth stencil texture from the depth stencil view.
	ID3D11Resource* tt;
	p_DepthStencilView->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt,&dssrvdesc, &m_DepthStencilResourceView);
	tt = nullptr;
	if(FAILED(result))
		return result;

	dssrvdesc.Format = desc.Format;
	// Make the diffuse texture from the render target.	
	m_RenderTargets[0]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_DiffuseSRV);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the normal texture from the render target.
	m_RenderTargets[1]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_NormalSRV);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the world position texture from the render target.
	m_RenderTargets[2]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_wPositionSRV);
	tt = nullptr;
	if(FAILED(result))
		return result;

	// Make the final texture from the render target.
	m_RenderTargets[3]->GetResource(&tt);
	result = m_Device->CreateShaderResourceView(tt, &dssrvdesc, &m_LightSRV);
	tt = nullptr;
	if(FAILED(result))
		return result;

	return result;
}

void DeferredRenderer::createConstantBuffer(int nrLights)
{
	// Create constant buffer for the lights
	BufferDescription sbdesc;
	sbdesc.initData = nullptr;
	sbdesc.numOfElements = 1;
	sbdesc.sizeOfElement = sizeof(Light);
	sbdesc.type = CONSTANT_BUFFER_ALL;
	sbdesc.usage = BUFFER_DEFAULT;
	m_LightBuffer = new Buffer();
	m_LightBuffer->initialize(m_Device, m_DeviceContext, sbdesc);

	cBuffer cb;
	cb.view = *m_ViewMatrix;
	cb.proj = *m_ProjectionMatrix;
	cb.campos = *m_CameraPosition;
	cb.nrLights = nrLights;

	BufferDescription cbdesc;
	cbdesc.initData = &cb;
	cbdesc.numOfElements = 1;
	cbdesc.sizeOfElement = sizeof(cBuffer);
	cbdesc.type = CONSTANT_BUFFER_ALL;
	cbdesc.usage = BUFFER_DEFAULT;
	m_ConstantBuffer = new Buffer();
	m_ConstantBuffer->initialize(m_Device, m_DeviceContext, cbdesc);
}

void DeferredRenderer::clearRenderTargets()
{
	float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	for(unsigned int i = 0; i < m_numRenderTargets; i++)
	{
		if(i == 1)
		{
			color[0] = 0.5f;
			color[1] = 0.5f;
			color[2] = 0.5f;
		}
		else if (i == 2)
		{
			color[0] = 1.0f;
			color[1] = 1.0f;
			color[2] = 1.0f;
		}
		else
		{
			color[0] = 0.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
		}
		m_DeviceContext->ClearRenderTargetView(m_RenderTargets[i], color);
	}
}
