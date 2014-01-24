#pragma once

#include <DirectXMath.h>
#include <string>
#include <vector>
#include <algorithm>

#include "Buffer.h"

struct Particle
{
	DirectX::XMFLOAT4 Position; //pos from the syspos, in cm
	DirectX::XMFLOAT4 Velocity;
	DirectX::XMFLOAT4 color;
	float sizeX;
	float sizeY;
	float life; //Life for a particle to live before taken away, in s
	float lifeMax;

	Particle(DirectX::XMFLOAT3 p_Position, DirectX::XMFLOAT3 p_Velocity, DirectX::XMFLOAT4 p_Color,
		float p_SizeX, float p_SizeY, float p_Life, float p_LifeMax)
	{
		Position = DirectX::XMFLOAT4(p_Position.x, p_Position.y, p_Position.z, 1.0f);
		Velocity = DirectX::XMFLOAT4(p_Velocity.x, p_Velocity.y, p_Velocity.z, 1.0f);
		color = p_Color;
		sizeX	= p_SizeX;
		sizeY	= p_SizeY;
		life	= p_Life;
		lifeMax = p_LifeMax;
	}
};

class ParticleSystem
{
private:
	std::vector<Particle>	m_ParticlesToSys;
	DirectX::XMFLOAT4	m_SysPosition; //world pos, in cm
	std::string			m_SysType;

	float				m_CurrentParticleCount;
	float				m_AccumulatedTime;

	//Buffer				m_Buffer; //probably getting fixed out in the graphic class
	//Shader				m_Shader; //probably getting fixed out in the graphic class

	//Header
	std::string			m_SysName;
	std::string			m_TextureFilePath;
	float				m_TextureU; 
	float				m_TextureV;	
	float				m_ParticleMaxLife;

	unsigned int		m_MaxParticles;
	DirectX::XMFLOAT3	m_ParticlePositionDeviation;
	DirectX::XMFLOAT3	m_VelocityDeviation;
	DirectX::XMFLOAT4	m_ParticleColorDeviation;
	float				m_SizeX;
	float				m_SizeY;
	float				m_ParticlesPerSecound;

public:
	ParticleSystem();
	~ParticleSystem();

	bool loadParticleSystemFromFile(const char* p_filename);

	void init();
	void update(float p_DeltaTime, ID3D11DeviceContext* p_DeviceContext);

	void render();
	

private:
	void emitNewParticles(float p_DeltaTime);
	void killOldParticles();
	void updateParticles(float p_DeltaTime);
	void updateBuffers(ID3D11DeviceContext* p_DeviceContext);
};

