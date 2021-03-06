#include "Body.h"
#include "PhysicsExceptions.h"

using namespace DirectX;

Body::BodyHandle Body::m_NextHandle = 1;

Body::BodyHandle Body::getNextHandle()
{
	return m_NextHandle++;
}
void Body::resetBodyHandleCounter()
{
	m_NextHandle = 1;
}

Body::Body(float p_mass, BoundingVolume::ptr p_BoundingVolume, bool p_IsImmovable, bool p_IsEdge)
	: m_Handle(getNextHandle())
{
	if(!p_BoundingVolume)
		m_Position = XMFLOAT4(0.f, 0.f, 0.f, 1.f);
	else
	{
		m_Volumes.push_back(std::move(p_BoundingVolume));
		m_Volumes.at(0)->setBodyHandle(m_Handle);
		m_Position = m_Volumes.at(0)->getPosition();
		updateSurroundingSphere(m_Volumes.back()->getSurroundingSphere());
	}
		

	m_Mass				= p_mass;	
	m_NetForce			= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_Velocity			= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_Acceleration		= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_LastAcceleration	= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_AvgAcceleration	= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_NewAcceleration	= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_Gravity			= 0.f;
	m_InAir				= true;
	m_OnSomething		= false;
	m_IsImmovable		= p_IsImmovable;
	m_IsEdge			= p_IsEdge;
	m_Landed			= false;

	m_ForceCollisionNormal	= false;
}

Body::Body(Body &&p_Other)
	: m_Handle(p_Other.m_Handle),
	  m_Volumes(std::move(p_Other.m_Volumes)),
	  m_Mass(p_Other.m_Mass),
	  m_Position(p_Other.m_Position),
	  m_NetForce(p_Other.m_NetForce),
	  m_Velocity(p_Other.m_Velocity),
	  m_Acceleration(p_Other.m_Acceleration),
	  m_LastAcceleration(p_Other.m_LastAcceleration),
	  m_AvgAcceleration(p_Other.m_AvgAcceleration),
	  m_NewAcceleration(p_Other.m_NewAcceleration),
	  m_Gravity(p_Other.m_Gravity),
	  m_InAir(p_Other.m_InAir),
	  m_OnSomething(p_Other.m_OnSomething),
	  m_IsImmovable(p_Other.m_IsImmovable),
	  m_IsEdge(p_Other.m_IsEdge),
	  m_Landed(p_Other.m_Landed),
	  m_ForceCollisionNormal(p_Other.m_ForceCollisionNormal),
	  m_SurroundingSphere(p_Other.m_SurroundingSphere)
{}

Body& Body::operator=(Body&& p_Other)
{
	std::swap(m_Handle, p_Other.m_Handle);
	std::swap(m_Volumes, p_Other.m_Volumes);
	std::swap(m_Mass, p_Other.m_Mass);
	std::swap(m_Position, p_Other.m_Position);
	std::swap(m_NetForce, p_Other.m_NetForce);
	std::swap(m_Velocity, p_Other.m_Velocity);
	std::swap(m_Acceleration, p_Other.m_Acceleration);
	std::swap(m_LastAcceleration, p_Other.m_LastAcceleration);
	std::swap(m_AvgAcceleration, p_Other.m_AvgAcceleration);
	std::swap(m_NewAcceleration, p_Other.m_NewAcceleration);
	std::swap(m_Gravity, p_Other.m_Gravity);
	std::swap(m_InAir, p_Other.m_InAir);
	std::swap(m_OnSomething, p_Other.m_OnSomething);
	std::swap(m_IsImmovable, p_Other.m_IsImmovable);
	std::swap(m_IsEdge, p_Other.m_IsEdge);
	std::swap(m_Landed, p_Other.m_Landed);
	std::swap(m_ForceCollisionNormal, p_Other.m_ForceCollisionNormal);
	std::swap(m_SurroundingSphere, p_Other.m_SurroundingSphere);

	return *this;
}

Body::~Body()
{
}

void Body::addForce(XMFLOAT4 p_Force)
{
	XMVECTOR tempForce, tempNetForce;
	tempForce = XMLoadFloat4(&p_Force);
	tempNetForce = XMLoadFloat4(&m_NetForce);

	tempNetForce += tempForce;

	XMStoreFloat4(&m_NetForce, tempNetForce);
}

void Body::addImpulse(DirectX::XMFLOAT4 p_Impulse)
{
	if (m_Mass == 0.f)
		return;

	XMVECTOR vVelocity = XMLoadFloat4(&m_Velocity);
	XMVECTOR impulse = XMLoadFloat4(&p_Impulse);
	vVelocity += impulse / m_Mass;
	XMStoreFloat4(&m_Velocity, vVelocity);
}

void Body::addVolume(BoundingVolume::ptr p_Volume)
{
	p_Volume->setBodyHandle(m_Handle);
	p_Volume->setIDInBody(m_Volumes.size());
	m_Volumes.push_back(std::move(p_Volume));
	updateSurroundingSphere(m_Volumes.back()->getSurroundingSphere());
}

void Body::update(float p_DeltaTime)
{
	if(m_IsImmovable)
		return;

	m_LastAcceleration = m_AvgAcceleration;

	XMFLOAT4 relativePos = XMFLOAT4(0.f, 0.f, 0.f, 0.f);	// cm

	relativePos.x = m_Velocity.x * p_DeltaTime + (0.5f * m_LastAcceleration.x * (p_DeltaTime*p_DeltaTime));
	relativePos.y = m_Velocity.y * p_DeltaTime + (0.5f * m_LastAcceleration.y * (p_DeltaTime*p_DeltaTime));
	relativePos.z = m_Velocity.z * p_DeltaTime + (0.5f * m_LastAcceleration.z * (p_DeltaTime*p_DeltaTime));

	m_Position.x += relativePos.x;
	m_Position.y += relativePos.y;
	m_Position.z += relativePos.z;

	m_NewAcceleration = calculateAcceleration();

	XMVECTOR tempAvg, tempNew, tempLast;	// m/s^2
	tempAvg = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	
	tempNew = XMLoadFloat4(&m_NewAcceleration);
	tempLast = XMLoadFloat4(&m_LastAcceleration);

	tempAvg = (tempLast + tempNew) * 0.5f;

	XMStoreFloat4(&m_AvgAcceleration, tempAvg);

	m_Velocity.x += m_AvgAcceleration.x * p_DeltaTime;
	m_Velocity.y += m_AvgAcceleration.y * p_DeltaTime;
	m_Velocity.z += m_AvgAcceleration.z * p_DeltaTime;

	updateBoundingVolumePosition(relativePos);
}

void Body::updateBoundingVolumePosition(DirectX::XMFLOAT4 p_Position)
{
	XMMATRIX matTrans = XMMatrixTranslation(p_Position.x, p_Position.y, p_Position.z);
	XMFLOAT4X4 tempTrans;

	XMStoreFloat4x4(&tempTrans, matTrans);

	for(auto &v : m_Volumes)
		v->updatePosition(tempTrans);

	m_SurroundingSphere.setPosition(XMLoadFloat4(&m_Position));
}

XMFLOAT4 Body::calculateAcceleration()
{
	XMFLOAT4 acc;	// m/s^2

	if(m_Mass == 0)
	{
		return XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	}
	else
	{
		acc.x = m_NetForce.x/m_Mass;
		acc.y = m_NetForce.y/m_Mass - m_Gravity;
		acc.z = m_NetForce.z/m_Mass;
	}


	acc.w = 0.f;
	return acc;
}

void Body::updateSurroundingSphere(const Sphere* p_ChangedVolumeSphere)
{
	m_SurroundingSphere.setPosition(XMLoadFloat4(&m_Position));
	const float currentRadius = m_SurroundingSphere.getRadius();
	const float changedRadius = p_ChangedVolumeSphere->getRadius();
	
	const XMFLOAT4 changedSpherePos = p_ChangedVolumeSphere->getPosition();
	const XMFLOAT3 diffPos(
		m_Position.x - changedSpherePos.x,
		m_Position.y - changedSpherePos.y,
		m_Position.z - changedSpherePos.z);

	const float diffSq = diffPos.x * diffPos.x + diffPos.y * diffPos.y + diffPos.z * diffPos.z;
	const float diff = sqrtf(diffSq);
	
	if (currentRadius < diff + changedRadius)
	{
		m_SurroundingSphere.setRadius(diff + changedRadius);
	}
}

void Body::setGravity(float p_Gravity)
{
	m_Gravity = p_Gravity;
}

bool Body::getInAir()
{
	return m_InAir;
}

void Body::setInAir(bool p_bool)
{
	m_InAir = p_bool;
}

bool Body::getOnSomething()
{
	return m_OnSomething;
}

void Body::setOnSomething(bool p_bool)
{
	m_OnSomething = p_bool;
}

void Body::setRotation(XMMATRIX const &p_Rotation)
{
	for(auto &v : m_Volumes)
		v->setRotation(p_Rotation);
}

bool Body::getLanded()
{
	return m_Landed;
}

void Body::setLanded(bool p_bool)
{
	m_Landed = p_bool;
}

bool Body::getIsImmovable() const
{
	return m_IsImmovable;
}
bool Body::getIsEdge() const
{
	return m_IsEdge;
}

void Body::setCollisionResponse(bool p_State)
{
	for(unsigned int i = 0; i < m_Volumes.size(); i++)
		m_Volumes.at(i)->setCollisionResponse(p_State);
}

void Body::setCollisionResponse(unsigned int p_Volume, bool p_State)
{
	m_Volumes.at(p_Volume)->setCollisionResponse(p_State);
}

bool Body::getCollisionResponse(unsigned int p_Volume)
{
	return m_Volumes.at(p_Volume)->getCollisionResponse();
}

BoundingVolume* Body::getVolume()
{
	return m_Volumes.at(0).get();
}

const BoundingVolume* Body::getVolume() const
{
	return m_Volumes.at(0).get();
}

BoundingVolume* Body::getVolume(unsigned p_Volume)
{
	if (p_Volume >= m_Volumes.size())
		return &m_SurroundingSphere;

	return m_Volumes.at(p_Volume).get();
}

unsigned int Body::getVolumeListSize()
{
	return m_Volumes.size();
}

void Body::setVolumePosition(unsigned p_Volume, DirectX::XMVECTOR const &p_Position)
{
	m_Volumes.at(p_Volume)->setPosition(p_Position);
	updateSurroundingSphere(m_Volumes.at(p_Volume)->getSurroundingSphere());
}

XMFLOAT4 Body::getVelocity()
{
	return m_Velocity;
}

void Body::setVelocity(XMFLOAT4 p_Velocity)
{
	m_Velocity.x = p_Velocity.x;
	m_Velocity.y = p_Velocity.y;
	m_Velocity.z = p_Velocity.z;
}

XMFLOAT4 Body::getPosition()
{
	return m_Position;
}

void Body::setPosition(XMFLOAT4 p_Position)
{
	XMFLOAT4 diffPos(p_Position.x - m_Position.x, p_Position.y - m_Position.y, p_Position.z - m_Position.z, p_Position.w - m_Position.w);
	m_Position = p_Position;
	updateBoundingVolumePosition(diffPos);
}

DirectX::XMFLOAT4 Body::getNetForce()
{
	return m_NetForce;
}

DirectX::XMFLOAT4 Body::getACC()
{
	return m_NewAcceleration;
}

float Body::getGravity() const
{
	return m_Gravity;
}

DirectX::XMFLOAT4 Body::getLastACC()
{
	return m_LastAcceleration;
}

void Body::resetForce()
{
	m_NetForce			= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_Velocity			= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_Acceleration		= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_AvgAcceleration	= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	m_NewAcceleration	= XMFLOAT4(0.f, 0.f, 0.f, 0.f);
}

void Body::setForceCollisionNormal(bool p_Bool)
{
	m_ForceCollisionNormal = p_Bool;
}
bool Body::getForceCollisionNormal() const
{
	return m_ForceCollisionNormal;
}

const Sphere* Body::getSurroundingSphere() const
{
	return getVolume()->getSurroundingSphere();
}
