#include "DeferredRenderer.h"
#include <fstream>
DeferredRenderer::DeferredRenderer(void)
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		m_RenderTargets[i] = nullptr;
	}

	m_DiffuseSRV = nullptr;
	m_NormalSRV = nullptr;
	m_LightSRV = nullptr;
	m_wPositionSRV = nullptr;

	m_Sampler = nullptr;
	m_LightShader = nullptr;
	m_ConstantBuffer = nullptr;
	m_AllLightBuffer = nullptr;

	m_CameraPosition = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;

	m_speed = 1.0f;
}

DeferredRenderer::~DeferredRenderer(void)
{
	m_Objects.clear();
	m_Lights.clear();

	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_DepthStencilView = nullptr;

	for(int i = 0; i < m_numRenderTargets; i++)
	{
		SAFE_RELEASE(m_RenderTargets[i]);
	}

	SAFE_RELEASE(m_DiffuseSRV);
	SAFE_RELEASE(m_NormalSRV);
	SAFE_RELEASE(m_LightSRV);
	SAFE_RELEASE(m_wPositionSRV);

	SAFE_RELEASE(m_Sampler);
	SAFE_DELETE(m_LightShader);
	SAFE_DELETE(m_ConstantBuffer);
	SAFE_DELETE(m_AllLightBuffer);
	SAFE_DELETE(m_PointLightBuffer);

	m_CameraPosition = nullptr;
	m_ViewMatrix = nullptr;
	m_ProjectionMatrix = nullptr;
}

void DeferredRenderer::initialize(ID3D11Device* p_Device, ID3D11DeviceContext* p_DeviceContext,
								  ID3D11DepthStencilView *p_DepthStencilView,
								  unsigned int p_screenWidth, unsigned int p_screenHeight,
								  DirectX::XMFLOAT3 *p_CameraPosition, DirectX::XMFLOAT4X4 *p_ViewMatrix, DirectX::XMFLOAT4X4 *p_ProjectionMatrix)
{
	m_Device			= p_Device;
	m_DeviceContext		= p_DeviceContext;
	m_DepthStencilView	= p_DepthStencilView;

	m_CameraPosition	= p_CameraPosition;
	m_ViewMatrix		= p_ViewMatrix;
	m_ProjectionMatrix	= p_ProjectionMatrix;

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

	//Compile shader for the light pass
	D3D11_INPUT_ELEMENT_DESC shaderDesc[] = 
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,	  0},
		{"LPOSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA,  1},
		{"COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"DIRECTION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"ANGLE",		0, DXGI_FORMAT_R32G32_FLOAT,	1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"RANGE",		0, DXGI_FORMAT_R32_FLOAT,		1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TYPE",		0, DXGI_FORMAT_R32_FLOAT,		1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};



	m_LightShader = new Shader();
	m_LightShader->initialize(m_Device,m_DeviceContext, 7);
	m_LightShader->compileAndCreateShader(L"../../Graphics/Source/DeferredShaders/LightPassV2Spotlight.hlsl","PointLightVS","vs_5_0",VERTEX_SHADER, shaderDesc);
	m_LightShader->compileAndCreateShader(L"../../Graphics/Source/DeferredShaders/LightPassV2Spotlight.hlsl","PointLightPS","ps_5_0",PIXEL_SHADER, nullptr);

	// Create sampler state and blend state for Alpha rendering.
	createSamplerState();
	createBlendStates();

	// TEMPORARY -----------------------------------------------------------
	// Make the light
	Light testLight(	
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), 
		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f), 
		DirectX::XMFLOAT2(0.5f, 1.0f),  
		50.0f,
		1
		);
	addLight(testLight);

	xx = 1;
	yy = 1;
	zz = 1;
	int minX,minY,minZ,maxX,maxY,maxZ;
	minX = minY = minZ = -20;
	maxX = maxY = maxZ = 20;
	float dx,dy,dz, cx,cy,cz;
	dx = (float)(abs(maxX) + abs(minX))/((xx - 1 <= 0) ? 1 : xx - 1);
	dy = (float)(abs(maxY) + abs(minY))/((yy - 1 <= 0) ? 1 : xx - 1);
	dz = (float)(abs(maxZ) + abs(minZ))/((zz - 1 <= 0) ? 1 : xx - 1);

	/*for(int x= 0; x < xx; x++)
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
	}*/

	//This buffer is supposed to be moved to non temporary code
	BufferDescription cbdesc;
	cbdesc.initData = m_Lights.data();
	cbdesc.numOfElements = xx*yy*zz;
	cbdesc.sizeOfElement = sizeof(Light);
	cbdesc.type = VERTEX_BUFFER;
	cbdesc.usage = BUFFER_DEFAULT;
	m_AllLightBuffer = new Buffer();
	m_AllLightBuffer->initialize(m_Device, m_DeviceContext, cbdesc);

	ModelLoader modL;
	modL.loadFile("../../Graphics/Resources/spotlight +Z openGL.tx"); //spherex2
	const std::vector<std::vector<ModelLoader::IndexDesc>>& indices = modL.getIndices();
	const std::vector<DirectX::XMFLOAT3>& vertices = modL.getVertices();
	std::vector<DirectX::XMFLOAT3> temp;
	unsigned int nrIndices = indices.at(0).size();
	for(unsigned int i = 0; i < nrIndices; i++)
	{
	temp.push_back(vertices.at(indices.at(0).at(i).m_Vertex));
	}
	
	cbdesc.initData = temp.data();
	cbdesc.numOfElements = nrIndices;
	cbdesc.sizeOfElement = sizeof(DirectX::XMFLOAT3);
	cbdesc.type = VERTEX_BUFFER;
	cbdesc.usage = BUFFER_DEFAULT;
	m_PointLightBuffer = new Buffer();
	m_PointLightBuffer->initialize(m_Device, m_DeviceContext, cbdesc);
	temp.clear();
	modL.clear();
	//m_lightBufferSRV = m_AllLightBuffer->CreateBufferSRV(m_AllLightBuffer->getBufferPointer());

	m_TextureLoader = new TextureLoader(m_Device, m_DeviceContext);
	m_Specular = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/diff.jpg");
	m_Diffuse = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/uv alpha.png");
	m_NormalMap = m_TextureLoader->createTextureFromFile("../../Graphics/Resources/Cube1_NRM_RGB.jpg");


	D3D11_DEPTH_STENCIL_DESC dsdesc;
	ZeroMemory( &dsdesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	dsdesc.DepthEnable = true;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	HRESULT hr = m_Device->CreateDepthStencilState(&dsdesc, &m_DepthState);

	D3D11_RASTERIZER_DESC rdesc;
	ZeroMemory( &rdesc, sizeof( D3D11_RASTERIZER_DESC ) );
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_FRONT;
	//rdesc.FrontCounterClockwise = TRUE;
	m_Device->CreateRasterizerState(&rdesc,&m_RasterState);

	// TEMPORARY ----------------------------------------------------------------
	createConstantBuffer(xx*yy*zz);
}

void DeferredRenderer::renderDeferred()
{
	/*float dt = 0.016f;
	for(unsigned int i = 0; i < m_Lights.size();i++)
	{

	if(i%2 == 0)
	m_Lights.at(i).lightPos.y += m_speed * dt;
	else
	m_Lights.at(i).lightPos.y -= m_speed * dt;	
	}
	if((m_Lights.at(0).lightPos.y > 20) ||
	(m_Lights.at(0).lightPos.y < -20))
	m_speed *= -1.0f;*/


	// Clear render targets.
	clearRenderTargets();

	//// Update constant buffer
	//updateConstantBuffer(xx*yy*zz);
	//updateLightBuffer();

	// Render
	renderGeometry();

	// Update constant buffer. ## REMOVE WHEN NINJA KICK IS REMOVED. ##
	updateConstantBuffer(xx*yy*zz);
	updateLightBuffer();

	renderLighting();

	m_Objects.clear();
	//m_Lights.clear(); For debug this is not active
}

void DeferredRenderer::renderGeometry()
{
	unsigned int nrRT = 3;

	// Set the render targets.
	m_DeviceContext->OMSetRenderTargets(nrRT, m_RenderTargets, m_DepthStencilView);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// The textures will be needed to be grabbed from the model later.
	ID3D11ShaderResourceView *srvs[] = { m_Diffuse, m_NormalMap, m_Specular };
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0};

	m_ConstantBuffer->setBuffer(0);
	m_DeviceContext->PSSetShaderResources(0, 3, srvs);
	//m_DeviceContext->PSSetSamplers(0,1,&m_Sampler);

	for(unsigned int i = 0; i < m_Objects.size();i++)
	{
		updateConstantBuffer(int(i));
		// Send stuff.
		// The update of the sub resource has to be done externally.
		m_Objects.at(i).m_Buffer->setBuffer(0);
		// Set shader.
		m_Objects.at(i).m_Shader->setShader();
		m_Objects.at(i).m_Shader->setBlendState(m_BlendState2);

		m_DeviceContext->Draw(m_Objects.at(i).m_Buffer->getNumOfElements(), 0);
		m_Objects.at(i).m_Buffer->unsetBuffer(0);
		m_Objects.at(i).m_Shader->unSetShader();
	}

	m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	//m_DeviceContext->PSSetSamplers(0,0,0);
	m_ConstantBuffer->unsetBuffer(0);

	// Unset render targets.
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::renderLighting()
{
	unsigned int activeRenderTarget	= 3;
	unsigned int nrRT = 1;

	// Collect the shader resources in an array and create a clear array.
	ID3D11ShaderResourceView *srvs[] = { m_wPositionSRV,
										 m_NormalSRV,
										 m_DiffuseSRV };
	ID3D11ShaderResourceView *nullsrvs[] = {0,0,0,0};

	// Set texture sampler.
	m_DeviceContext->PSSetSamplers(0,1,&m_Sampler);
	m_LightShader->setBlendState(m_BlendState);

	m_DeviceContext->PSSetShaderResources(0, 3, srvs);

	////Select the third render target[3]
	m_DeviceContext->OMSetRenderTargets(nrRT, &m_RenderTargets[activeRenderTarget], m_DepthStencilView); 
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_DeviceContext->RSSetState(m_RasterState);
	m_DeviceContext->OMSetDepthStencilState(m_DepthState,0);

	//Set constant data
	m_ConstantBuffer->setBuffer(0);

	UINT Offsets[2] = {0,0};
	ID3D11Buffer * buffers[] = {m_PointLightBuffer->getBufferPointer(),
								m_AllLightBuffer->getBufferPointer()};
	UINT Stride[2] = {sizeof(DirectX::XMFLOAT3),
						sizeof(Light)};

	m_DeviceContext->IASetVertexBuffers(0,2,buffers,Stride, Offsets);


	//set shader
	m_LightShader->setShader();
	m_DeviceContext->DrawInstanced(m_PointLightBuffer->getNumOfElements(),
									m_AllLightBuffer->getNumOfElements(),
									0,0);
	//m_DeviceContext->Draw(6, 0);

	//unset resources	
	m_LightShader->unSetShader();

	m_ConstantBuffer->unsetBuffer(0);
	m_DeviceContext->IASetVertexBuffers(0,0,0,0, 0);

	m_DeviceContext->RSSetState(0);
	m_DeviceContext->OMSetDepthStencilState(0,0);

	m_DeviceContext->PSSetSamplers(0,0,0);
	m_LightShader->setBlendState(0);

	m_DeviceContext->PSSetShaderResources(0, 3, nullsrvs);
	m_DeviceContext->OMSetRenderTargets(0, 0, 0);
}

void DeferredRenderer::addRenderable(Renderable p_renderable)
{
	m_Objects.push_back(p_renderable);
}

void DeferredRenderer::addLight(Light p_Light)
{
	m_Lights.push_back(p_Light);

	/*switch(p_Light.lightType)
	{
	case 0:
	m_PointLights.push_back(p_Light);
	break;
	case 1:
	m_SpotLights.push_back(p_Light);
	break;
	case 2:
	m_DirectionalLights.push_back(p_Light);
	break;
	default:
	break;
	}*/
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
}

void DeferredRenderer::updateLightBuffer()
{
	m_DeviceContext->UpdateSubresource(m_AllLightBuffer->getBufferPointer(), NULL,NULL, m_Lights.data(),NULL,NULL);
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

	srvt0 = srvt1 = srvt2 = srvt3 = nullptr;
	// Done with the render targets.
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
	sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc   = D3D11_COMPARISON_NEVER;
	sd.MinLOD			= 0;
	sd.MaxLOD			= D3D11_FLOAT32_MAX;

	m_Device->CreateSamplerState( &sd, &m_Sampler );
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
		bd.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[i].DestBlend =  D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	}
	m_Device->CreateBlendState(&bd, &m_BlendState2);
}