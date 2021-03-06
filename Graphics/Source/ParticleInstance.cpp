#include "ParticleInstance.h"
#include <functional>
#include <algorithm>
#include <chrono>

ParticleInstance::ParticleInstance()
{
	m_SysPosition = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f); // change pos in the "GameLogic.cpp - LoadSandBox()" createParticles for local play
	m_SysRotation = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	m_AccumulatedTime = 0.f;
	m_SysLife = 0.f;
	m_SysMaxLife = -1.f;
	m_Seppuku = false;
}

ParticleInstance::~ParticleInstance()
{
}

void ParticleInstance::init(std::shared_ptr<Buffer> p_ConstBuffer, std::shared_ptr<Buffer> p_ParticleBuffer, ParticleEffectDefinition::ptr p_ParticleEffectDefinition)
{
	m_ConstBuffer = p_ConstBuffer;
	m_ParticleBuffer = p_ParticleBuffer;
	m_ParticleEffectDef = p_ParticleEffectDefinition;
	m_ParticleList.reserve(m_ParticleEffectDef->maxParticles);

	m_RandomEngine.seed((unsigned long)std::chrono::system_clock::now().time_since_epoch().count());
	
	if (m_ParticleEffectDef->particleColorBase.x != -1)
	{	
		m_SysBaseColor = DirectX::XMFLOAT4(m_ParticleEffectDef->particleColorBase.x,
										   m_ParticleEffectDef->particleColorBase.y,
										   m_ParticleEffectDef->particleColorBase.z,
										   m_ParticleEffectDef->particleColorBase.w);
	}
	if (m_ParticleEffectDef->sysMaxLife > 0)
	{
		m_SysMaxLife = m_ParticleEffectDef->sysMaxLife;
	}
}


void ParticleInstance::update(float p_DeltaTime) 
{
	if (m_SysMaxLife > 0)
	{
		m_SysLife += p_DeltaTime;
		
		if (m_SysLife >= m_SysMaxLife)
		{
			setSeppuku(true);
		}	
	}
	
	killOldParticles();

	if (m_SysLife <= m_SysMaxLife || m_SysMaxLife == -1)
	{
		emitNewParticles(p_DeltaTime);
	}
	
	updateParticles(p_DeltaTime);
}

void ParticleInstance::killOldParticles()
{
	//Will go thou the list of particles in the system and remove any particle that are to old

	auto removeIt = std::remove_if(m_ParticleList.begin(),m_ParticleList.end(), [&] (Particle &p) 
	{
		return p.life >= p.maxLife;
	});
	m_ParticleList.erase(removeIt, m_ParticleList.end());

}

void ParticleInstance::updateParticles(float p_DeltaTime)
{
	//Update the position of every particle in the system by its velocity and based on the delta time
	for(auto& part : m_ParticleList)
	{
		part.shaderData.position = DirectX::XMFLOAT3(
			(part.shaderData.position.x + part.velocity.x * p_DeltaTime),
			(part.shaderData.position.y + part.velocity.y * p_DeltaTime),
			(part.shaderData.position.z + part.velocity.z * p_DeltaTime));
		
		part.shaderData.color.w = m_ParticleEffectDef->particleColorBase.w * (1 - (part.life/part.maxLife));
		
		part.life += p_DeltaTime;
	}
}

void ParticleInstance::emitNewParticles(float p_DeltaTime)
{
	DirectX::XMFLOAT3 tempPos = DirectX::XMFLOAT3(m_SysPosition.x, m_SysPosition.y, m_SysPosition.z);
	DirectX::XMFLOAT4 tempColor = m_SysBaseColor;
	
	m_AccumulatedTime += p_DeltaTime;

	const float timePerParticle = 1.f / m_ParticleEffectDef->particlesPerSec;
	//check if new particles are to be emitted or not
	while (m_AccumulatedTime > timePerParticle)
	{
		

		m_AccumulatedTime -= timePerParticle;

		if (m_ParticleList.size() >= m_ParticleEffectDef->maxParticles)
		{
			break;
		}

		//Velocity
		std::uniform_real_distribution<float> velDistributionX(-m_ParticleEffectDef->velocityDeviation.x, m_ParticleEffectDef->velocityDeviation.x);
		std::uniform_real_distribution<float> velDistributionY(-m_ParticleEffectDef->velocityDeviation.y, m_ParticleEffectDef->velocityDeviation.y);
		std::uniform_real_distribution<float> velDistributionZ(-m_ParticleEffectDef->velocityDeviation.z, m_ParticleEffectDef->velocityDeviation.z);
		DirectX::XMFLOAT3 randVel(
			m_ParticleEffectDef->velocitybase.x + velDistributionX(m_RandomEngine),		
			m_ParticleEffectDef->velocitybase.y + velDistributionY(m_RandomEngine),
			m_ParticleEffectDef->velocitybase.z + velDistributionZ(m_RandomEngine));

		DirectX::XMMATRIX tempRotationM = DirectX::XMMatrixRotationRollPitchYaw(m_SysRotation.y, 
																				m_SysRotation.x, 
																				m_SysRotation.z);
		DirectX::XMVECTOR tempVEC = DirectX::XMLoadFloat3(&randVel);

		tempVEC = DirectX::XMVector3Transform(tempVEC, tempRotationM);

		DirectX::XMStoreFloat3(&randVel, tempVEC);

		//Position
		std::uniform_real_distribution<float> posDistribution(-m_ParticleEffectDef->particlePositionDeviation, m_ParticleEffectDef->particlePositionDeviation);
		DirectX::XMFLOAT3 randPos(
			tempPos.x + posDistribution(m_RandomEngine),
			tempPos.y + posDistribution(m_RandomEngine),
			tempPos.z + posDistribution(m_RandomEngine));

		//Life
		std::uniform_real_distribution<float> lifeDistribution(-m_ParticleEffectDef->maxLifeDeviation, m_ParticleEffectDef->maxLifeDeviation);
		float randMaxLife = m_ParticleEffectDef->maxLife + lifeDistribution(m_RandomEngine);

		//Color
		std::uniform_real_distribution<float> oneToOneDistribution(-1.f, 1.f);
		DirectX::XMFLOAT4 randColorOffset(
			tempColor.x + oneToOneDistribution(m_RandomEngine) * m_ParticleEffectDef->particleColorDeviation.x,
			tempColor.y + oneToOneDistribution(m_RandomEngine) * m_ParticleEffectDef->particleColorDeviation.y,
			tempColor.z + oneToOneDistribution(m_RandomEngine) * m_ParticleEffectDef->particleColorDeviation.z,
			tempColor.w + oneToOneDistribution(m_RandomEngine) * m_ParticleEffectDef->particleColorDeviation.w);

		//Put all the new data for the new particle into one container
		Particle tempParticle(randPos, randVel, randColorOffset, m_ParticleEffectDef->size, 0.f, randMaxLife);

		//Add the new particle to the others in the same system
		m_ParticleList.push_back(tempParticle);
	}
}

void ParticleInstance::updateBuffers(ID3D11DeviceContext *p_DeviceContext, DirectX::XMFLOAT3 *p_CameraPosition,
									 DirectX::XMFLOAT4X4 *p_ViewMatrix, DirectX::XMFLOAT4X4 *p_ProjectionMatrix)
{
	particlecBuffer pcb;
	pcb.viewM = *p_ViewMatrix;
	pcb.projM = *p_ProjectionMatrix;
	pcb.cameraPos = DirectX::XMFLOAT4(p_CameraPosition->x, p_CameraPosition->y, p_CameraPosition->z, 1.f);
	pcb.size = m_ParticleEffectDef->size;
	p_DeviceContext->UpdateSubresource(m_ConstBuffer->getBufferPointer(),NULL,NULL, &pcb, NULL,NULL);

	D3D11_MAPPED_SUBRESOURCE resource = {};
	p_DeviceContext->Map(m_ParticleBuffer->getBufferPointer(), 0, D3D11_MAP_WRITE_DISCARD, NULL, &resource);
	ShaderParticle* mappedShaderParticle = (ShaderParticle*)resource.pData;
	for (const auto& part : m_ParticleList)
	{
		*mappedShaderParticle = part.shaderData;
		mappedShaderParticle++;
	}
	p_DeviceContext->Unmap(m_ParticleBuffer->getBufferPointer(), 0);
}

void ParticleInstance::render(ID3D11DeviceContext* p_Context, ID3D11BlendState* p_BlendState)
{
	p_Context->PSSetSamplers(0,1,&m_ParticleEffectDef->sampler);
	m_ParticleBuffer->setBuffer(0);
	m_ConstBuffer->setBuffer(1);

	m_ParticleEffectDef->shader->setShader();
	float data[] = { 1.0f, 1.0f, 1.f, 1.0f};
	m_ParticleEffectDef->shader->setBlendState(p_BlendState, data);

	ID3D11ShaderResourceView *srvs[] =  {
		m_ParticleEffectDef->diffuseTexture
	};
	p_Context->PSSetShaderResources(0, 1, srvs);

	p_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	p_Context->Draw(m_ParticleList.size(), 0);

	static ID3D11ShaderResourceView * const nullsrvs[] = {NULL};
	p_Context->PSSetShaderResources(0, 1, nullsrvs);

	m_ParticleEffectDef->shader->setBlendState(0, data);
	m_ParticleEffectDef->shader->unSetShader();
	m_ParticleBuffer->unsetBuffer(0);
	m_ConstBuffer->unsetBuffer(1);
	ID3D11SamplerState* noState = nullptr;
	p_Context->PSSetSamplers(0, 1, &noState);
}

void ParticleInstance::setPosition(DirectX::XMFLOAT4 p_NewPosition)
{
	m_SysPosition = p_NewPosition;
}

DirectX::XMFLOAT4X4 ParticleInstance::getWorldMatrix() const
{
	DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(m_SysPosition.x, m_SysPosition.y, m_SysPosition.z);
	DirectX::XMFLOAT4X4 worldF;
	DirectX::XMStoreFloat4x4(&worldF, world);
	return worldF;
}

DirectX::XMFLOAT3 ParticleInstance::getSysRotation() const
{
	return m_SysRotation;
}

void ParticleInstance::setSysRotation(DirectX::XMFLOAT3 p_NewSysRotation)
{
	m_SysRotation = p_NewSysRotation;
}

DirectX::XMFLOAT4 ParticleInstance::getSysBaseColor() const
{
	return m_SysBaseColor;
}

void ParticleInstance::setSysBaseColor(DirectX::XMFLOAT4 p_NewSysBaseColor)
{
	m_SysBaseColor = p_NewSysBaseColor;
}

bool ParticleInstance::getSeppuku() const
{
	return m_Seppuku;
}

void ParticleInstance::setSeppuku(bool p_DoSeppuku)
{
	m_Seppuku = p_DoSeppuku;
}