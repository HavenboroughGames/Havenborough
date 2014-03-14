#include "Physics.h"
#include "Collision.h"
#include "PhysicsLogger.h"
#include "PhysicsExceptions.h"

using namespace DirectX;

Physics::Physics(void)
	: m_GlobalGravity(30.f)
{}

Physics::~Physics()
{
	m_Bodies.clear();
	m_sphereBoundingVolume.clear();
}

IPhysics *IPhysics::createPhysics()
{
	return new Physics();
}

void IPhysics::deletePhysics(IPhysics* p_Physics)
{
	if (p_Physics)
	{
		PhysicsLogger::log(PhysicsLogger::Level::INFO, "Shutting down physics");
		p_Physics->releaseAllBoundingVolumes();
		delete p_Physics;
	}
}

Body* Physics::findBody(BodyHandle p_Body)
{
	for (Body& b : m_Bodies)
	{
		if (b.getHandle() == p_Body)
		{
			return &b;
		}
	}

	return nullptr;
}

void Physics::initialize(bool p_IsServer)
{
	PhysicsLogger::log(PhysicsLogger::Level::INFO, "Initializing physics");

	fillTriangleIndexList();
	m_LoadBVSphereTemplateOnce = true;
	m_IsServer = p_IsServer;
}

void Physics::update(float p_DeltaTime, unsigned p_FPSCheckLimit)
{
	int itr = 1;

	float timestep = 1 / (float)(p_FPSCheckLimit * 2.f); 

	if(timestep*2.f < p_DeltaTime)
	{
		itr = (int)ceil(fabs(p_DeltaTime / timestep - 0.5f)); // When the fps goes under this the game start to update physics more than once / frame.
		//itr = 1;

		if(itr > 2)
			itr = 2;
		else if(itr == 0)
			itr = 1;
						
		p_DeltaTime /= itr;
	}

	m_HitDatas.clear();
	unsigned int nrOfBodies = m_Bodies.size();
	for(int p = 0; p < itr; p++)
	{
		for(unsigned i = 0; i < nrOfBodies; i++)
		{
			Body& b = m_Bodies[i];

			if(b.getIsImmovable())
				continue;

			b.update(p_DeltaTime);

			b.setLanded(false);

			bool isOnGround = false;
			
			for (unsigned j = 0; j < nrOfBodies; j++)
			{
				if(i == j)
					continue;
				
				for(unsigned int k = 0; k < b.getVolumeListSize(); k++)
				{
					for(unsigned int l = 0; l < m_Bodies.at(j).getVolumeListSize(); l++)
					{
						HitData hit = Collision::boundingVolumeVsBoundingVolume(*b.getVolume(k), *m_Bodies.at(j).getVolume(l));
	
						if(hit.intersect)
						{
							if(isCameraPlayerCollision(b, m_Bodies.at(j)))
								break;

							if(k == 0 && hit.colType == Type::HULLVSSPHERE)
							{
								XMFLOAT4 fBodyPos = b.getPosition();
								XMFLOAT4 fVictimPos = m_Bodies.at(j).getPosition();
								Sphere s = ((Hull*)m_Bodies.at(j).getVolume(l))->getSphere();
								float r  = ((Sphere*)b.getVolume(0))->getRadius();
								if((s.getRadius() < 1.f && fVictimPos.y > fBodyPos.y - 0.35f && fVictimPos.y < fBodyPos.y))
								{
									//PhysicsLogger::log(PhysicsLogger::Level::INFO, "StepSize");
									setBodyForceCollisionNormal(b.getHandle(), m_Bodies.at(j).getHandle(), true);
								}
								else
									handleCollision(hit, i, k, j, l, isOnGround);
							}
							else
								handleCollision(hit, i, k, j, l, isOnGround);
						}
					}
				}
			}

			if(!m_IsServer)
			{
				b.setOnSomething(isOnGround);
				b.setInAir(!isOnGround);
			}
		}
	}
}

void Physics::handleCollision(HitData p_Hit, int p_Collider, int p_ColliderVolumeId, int p_Victim, int p_VictimVolumeID, bool &p_IsOnGround)
{
	Body& b = m_Bodies[p_Collider];
	Body& b1 = m_Bodies[p_Victim];
	p_Hit.collider = b.getHandle();
	p_Hit.IDInBody = p_ColliderVolumeId;
	p_Hit.collisionVictim = b1.getHandle();
	p_Hit.isEdge = b1.getIsEdge();
	m_HitDatas.push_back(p_Hit);

	if(!m_IsServer)
	{
		if(b.getCollisionResponse(p_ColliderVolumeId) && b1.getCollisionResponse(p_VictimVolumeID))
		{
			XMVECTOR temp;		// m
			XMFLOAT4 tempPos;	// m

			XMFLOAT4 vel = b.getVelocity();
			XMVECTOR vVel = XMLoadFloat4(&vel);

			XMVECTOR vNorm = Vector4ToXMVECTOR(&p_Hit.colNorm);
			XMVECTOR posNorm = vNorm;

			if (p_Hit.colNorm.y > 0.7f)
			{
				if(!b.getOnSomething())
				{
					b.setLanded(true);
				}

				p_IsOnGround = true;

				posNorm = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				vVel = vVel - XMVector3Dot(vVel, vNorm) / XMVector3Dot(posNorm, vNorm) * posNorm;
			}
			else
			{
				vVel -= XMVector4Dot(vVel, vNorm) * vNorm;
			}

			XMStoreFloat4(&vel, vVel);
			b.setVelocity(vel);


			temp = XMLoadFloat4(&b.getPosition()) + posNorm * p_Hit.colLength;
			XMStoreFloat4(&tempPos, temp);

			b.setPosition(tempPos);
		}
	}
}

void Physics::applyForce(BodyHandle p_Body, Vector3 p_Force)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to apply force on a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	XMFLOAT4 tempForce = Vector3ToXMFLOAT4(&p_Force, 0.f); // kg*m/s^2

	body->addForce(tempForce);
}

void Physics::applyImpulse(BodyHandle p_Body, Vector3 p_Impulse)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to apply impulse on a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	XMFLOAT4 fImpulse = Vector3ToXMFLOAT4(&p_Impulse, 0.f);
	body->addImpulse(fImpulse);
}

BodyHandle Physics::createSphere(float p_Mass, bool p_IsImmovable, Vector3 p_Position, float p_Radius)
{
	Vector3 convPosition = p_Position * 0.01f;	// m
	XMFLOAT4 tempPosition = Vector3ToXMFLOAT4(&convPosition, 1.f); // m

	Sphere* sphere = new Sphere(p_Radius / 100.f, tempPosition);

	return createBody(p_Mass, sphere, p_IsImmovable, false);
}

BodyHandle Physics::createAABB(float p_Mass, bool p_IsImmovable, Vector3 p_CenterPos, Vector3 p_Extents, bool p_IsEdge)
{
	Vector3 convPosition = p_CenterPos * 0.01f;	// m
	Vector3 convExtents = p_Extents * 0.01f;	// m

	XMFLOAT4 tempPos = Vector3ToXMFLOAT4(&convPosition, 1.f);	// m
	XMFLOAT4 tempExt = Vector3ToXMFLOAT4(&convExtents , 0.f);	// m

	AABB* aabb = new AABB(tempPos, tempExt);

	return createBody(p_Mass, aabb, p_IsImmovable, p_IsEdge);
}

BodyHandle Physics::createOBB(float p_Mass, bool p_IsImmovable, Vector3 p_CenterPos, Vector3 p_Extent, bool p_IsEdge)
{
	Vector3 convPosition = p_CenterPos * 0.01f;	// m
	Vector3 convExtents = p_Extent * 0.01f;	// m

	XMFLOAT4 tempPos	= Vector3ToXMFLOAT4(&convPosition, 1.f);	// m
	XMFLOAT4 tempExt	= Vector3ToXMFLOAT4(&convExtents, 0.f);	// m

	OBB *obb = new OBB(tempPos, tempExt);

	return createBody(p_Mass, obb, p_IsImmovable, p_IsEdge);
}

void Physics::addSphereToBody(BodyHandle p_BodyHandle, Vector3 p_Position, float p_Radius)
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to add a sphere to a non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	Vector3 convPosition = p_Position * 0.01f;	// m
	XMFLOAT4 tempPosition = Vector3ToXMFLOAT4(&convPosition, 1.f); // m

	Sphere* sphere = new Sphere(p_Radius / 100.f, tempPosition);

	body->addVolume(BoundingVolume::ptr(sphere));
}

void Physics::addOBBToBody(BodyHandle p_BodyHandle, Vector3 p_CenterPos, Vector3 p_Extents) 
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to add a OBB to a non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	Vector3 convPosition = p_CenterPos * 0.01f;	// m
	Vector3 convExtents = p_Extents * 0.01f;
	XMFLOAT4 tempPosition = Vector3ToXMFLOAT4(&convPosition, 1.f); // m
	XMFLOAT4 tempExt	= Vector3ToXMFLOAT4(&convExtents, 0.f);	// m

	OBB* obb = new OBB(tempPosition, tempExt);

	body->addVolume(BoundingVolume::ptr(obb));
}

BodyHandle Physics::createBVInstance(const char* p_VolumeID)
{
	std::vector<BVLoader::BoundingVolume> tempBV;
	for(auto& bv : m_TemplateBVList)
	{
		if(strcmp(bv.first.c_str(), p_VolumeID) == 0)
		{
			tempBV = bv.second;
			break;
		}
	}

	if(tempBV.empty())
	{	
		PhysicsLogger::log(PhysicsLogger::Level::ERROR_L, "Bounding Volume from template is empty");
		return (BodyHandle)0;
	}

	std::vector<Triangle> triangles;
	Triangle triangle;

	for(unsigned i = 0; i < tempBV.size() / 3; i++)
	{
		triangle.corners[0] = tempBV[i * 3].m_Postition;
		triangle.corners[1] = tempBV[i * 3 + 1].m_Postition;
		triangle.corners[2] = tempBV[i * 3 + 2].m_Postition;

		triangles.push_back(triangle);
	}

	Hull *hull = new Hull(triangles);

	return createBody(1.f, hull, true, false);

}

void Physics::releaseBody(BodyHandle p_Body)
{
	for (auto& body : m_Bodies)
	{
		if (body.getHandle() == p_Body)
		{
			std::swap(body, m_Bodies.back());
			m_Bodies.pop_back();
			return;
		}
	}
}

bool Physics::createBV(const char* p_VolumeID, const char* p_FilePath)
{
	if(!m_BVLoader.loadBinaryFile(p_FilePath))
	{
		PhysicsLogger::log(PhysicsLogger::Level::ERROR_L, "Loading Bounding Volume file error");
		return false;
	}
	std::vector<BVLoader::BoundingVolume> tempBV;
	tempBV = m_BVLoader.getBoundingVolumes();

	if(tempBV.empty())
	{
		PhysicsLogger::log(PhysicsLogger::Level::ERROR_L, "Bounding Volume from BVLoader is empty");
		return false;
	}

	for(unsigned i = 0; i < tempBV.size(); i++)
	{
		tempBV[i].m_Postition.x *= 0.01f;
		tempBV[i].m_Postition.y *= 0.01f;
		tempBV[i].m_Postition.z *= 0.01f; 
	}

	m_TemplateBVList.push_back(std::pair<std::string, std::vector<BVLoader::BoundingVolume>>(p_VolumeID, tempBV));
	m_BVLoader.clear();
	//PhysicsLogger::log(PhysicsLogger::Level::INFO, "CreateBV success");
	return true;
}

bool Physics::releaseBV(const char* p_VolumeID)
{
	for(auto bv = m_TemplateBVList.begin(); bv != m_TemplateBVList.end(); bv++)
	{
		const char* temp = bv->first.c_str();
		if(strcmp(temp, p_VolumeID) == 0)
		{
			m_TemplateBVList.erase(bv);
			return true;
		}
	}

	return false;
}

void Physics::releaseAllBoundingVolumes(void)
{
	m_Bodies.clear();
	m_Bodies.shrink_to_fit();
	Body b;
	b.resetBodyHandleCounter();
	m_sphereBoundingVolume.clear();
}

void Physics::setBodyScale(BodyHandle p_BodyHandle, Vector3 p_Scale)
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to set scale to a a non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	XMVECTOR scale = Vector3ToXMVECTOR(&p_Scale, 0.f);

	switch (body->getVolume()->getType())
	{
	case BoundingVolume::Type::AABBOX:
		((AABB*)body->getVolume())->scale(scale);
		break;

	case BoundingVolume::Type::HULL:
		((Hull*)body->getVolume())->scale(scale);
		break;

	case BoundingVolume::Type::OBB:
		((OBB*)body->getVolume())->scale(scale);
		break;

	case BoundingVolume::Type::SPHERE:
		((Sphere*)body->getVolume())->scale(scale);
		break;
	default:
		break;
	}
}

BodyHandle Physics::createBody(float p_Mass, BoundingVolume* p_BoundingVolume, bool p_IsImmovable, bool p_IsEdge)
{
	m_Bodies.emplace_back(p_Mass, BoundingVolume::ptr(p_BoundingVolume), p_IsImmovable, p_IsEdge);
	m_Bodies.back().setGravity(m_GlobalGravity);
	return m_Bodies.back().getHandle();
}

void Physics::fillTriangleIndexList()
{
	m_BoxTriangleIndex.push_back(XMFLOAT3(1, 0, 2));
	m_BoxTriangleIndex.push_back(XMFLOAT3(2, 3, 1));
	m_BoxTriangleIndex.push_back(XMFLOAT3(5, 1, 3));
	m_BoxTriangleIndex.push_back(XMFLOAT3(3, 7, 5));
	m_BoxTriangleIndex.push_back(XMFLOAT3(4, 5, 7));
	m_BoxTriangleIndex.push_back(XMFLOAT3(7, 6, 4));
	m_BoxTriangleIndex.push_back(XMFLOAT3(2, 0, 4));
	m_BoxTriangleIndex.push_back(XMFLOAT3(4, 6, 2));
	m_BoxTriangleIndex.push_back(XMFLOAT3(6, 7, 3));
	m_BoxTriangleIndex.push_back(XMFLOAT3(3, 2, 6));
	m_BoxTriangleIndex.push_back(XMFLOAT3(1, 5, 4));
	m_BoxTriangleIndex.push_back(XMFLOAT3(4, 0, 1));
}

void Physics::setGlobalGravity(float p_Gravity)
{
	m_GlobalGravity = p_Gravity;
}

void Physics::setBodyGravity(BodyHandle p_Body, float p_Gravity)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set gravity to a a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	body->setGravity(p_Gravity);
}

bool Physics::getBodyInAir(BodyHandle p_Body)
{
    Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to find if the body is in air from a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

    return body->getInAir();
}

bool Physics::getBodyOnSomething(BodyHandle p_Body)
{
    Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to find if body is on something in a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	return body->getOnSomething();
}

BoundingVolume* Physics::getVolume(BodyHandle p_Body)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to get a volume from a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	return body->getVolume();
}

HitData Physics::getHitDataAt(unsigned int p_Index)
{
	return m_HitDatas.at(p_Index);
}

unsigned int Physics::getHitDataSize()
{
	return m_HitDatas.size();
}

bool Physics::getBodyLanded(BodyHandle p_Body)
{
	Body *body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to get Landed from a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	return body->getLanded();
}

void Physics::setBodyCollisionResponse(BodyHandle p_Body, bool p_State)
{
	Body *body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set collision response for a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	body->setCollisionResponse(p_State);
}

void Physics::setBodyVolumeCollisionResponse(BodyHandle p_Body, int p_Volume, bool p_State)
{
	Body *body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set collision response for a volume in a non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	body->setCollisionResponse(p_Volume, p_State);
}


Vector3 Physics::getBodyPosition(BodyHandle p_Body)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to get position from non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	XMFLOAT4 temp = body->getPosition();	// m

	Vector3 tempvec3(temp.x * 100.f, temp.y * 100.f, temp.z * 100.f);	// cm

	return tempvec3;
}

Vector3 Physics::getBodySize(BodyHandle p_Body)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to get size on non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	Vector3 temp;
	float r;
	switch (body->getVolume()->getType())
	{
	case BoundingVolume::Type::AABBOX:
		temp = XMFLOAT4ToVector3(&((AABB*)body->getVolume())->getHalfDiagonal());
		break;
	case BoundingVolume::Type::SPHERE:
		r = ((Sphere*)body->getVolume())->getRadius();
		temp = Vector3(r,r,r);
		break;
	case BoundingVolume::Type::OBB:
		temp = XMFLOAT4ToVector3(&((OBB*)body->getVolume())->getExtents());
		break;
	case BoundingVolume::Type::HULL:
		temp = XMFLOAT4ToVector3(&((Hull*)body->getVolume())->getScale());
		break;
	default:
		temp = Vector3(0,0,0);
		break;
	}
	
	return temp * 100.f;
}

void Physics::setBodyPosition( BodyHandle p_Body, Vector3 p_Position)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set position on non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	Vector3 convPosition = p_Position * 0.01f;	// m
	XMFLOAT4 tempPosition = Vector3ToXMFLOAT4(&convPosition, 1.f);	// m

	body->setPosition(tempPosition);
}

void Physics::setBodyVolumePosition( BodyHandle p_Body, unsigned p_Volume, Vector3 p_Position)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set volume position on non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	Vector3 convPosition = p_Position * 0.01f;	// m
	body->setVolumePosition(p_Volume, Vector3ToXMVECTOR(&convPosition, 1.f));
}


void Physics::setBodyVelocity( BodyHandle p_Body, Vector3 p_Velocity)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set velocity on non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	Vector3 convVelocity = p_Velocity * 0.01f;	// m
	XMFLOAT4 tempPosition = Vector3ToXMFLOAT4(&convVelocity, 0.f);	// m

	body->setVelocity(tempPosition);
}

Vector3 Physics::getBodyVelocity(BodyHandle p_Body)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to get velocity from non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	XMFLOAT4 tempVel = body->getVelocity();

	return Vector3(tempVel.x, tempVel.y, tempVel.z) * 100.f;
}

void Physics::setBodyRotation( BodyHandle p_Body, Vector3 p_Rotation)
{
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(p_Rotation.y, p_Rotation.x, p_Rotation.z);
	setRotation(p_Body, rotation);
}

void Physics::setBodyRotationMatrix(BodyHandle p_Body, XMFLOAT4X4 p_Rotation)
{
	XMMATRIX rotation = XMLoadFloat4x4(&p_Rotation);
	setRotation(p_Body, rotation);
}

void Physics::setLogFunction(clientLogCallback_t p_LogCallback)
{
	PhysicsLogger::setLogFunction(p_LogCallback);
}

Triangle Physics::getTriangleFromBody(unsigned int p_BodyHandle, unsigned int p_TriangleIndex, int p_BoundingVolume)
{
	Body* body = findBody(p_BodyHandle);
	Triangle trig;
	if(!body)
		throw PhysicsException("Error! Trying to get a triangle from non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);
	
	BoundingVolume *volume = body->getVolume(p_BoundingVolume);

	switch (volume->getType())
	{
	case BoundingVolume::Type::AABBOX:
		{
			XMFLOAT3 triangleIndex = m_BoxTriangleIndex.at(p_TriangleIndex);
			Triangle triangle = Triangle(Vector4(((AABB*)volume)->getBoundWorldCoordAt((int)triangleIndex.x)) * 100.f,
										Vector4(((AABB*)volume)->getBoundWorldCoordAt((int)triangleIndex.y)) * 100.f,
										Vector4(((AABB*)volume)->getBoundWorldCoordAt((int)triangleIndex.z)) * 100.f);

			return triangle;
		}
	case BoundingVolume::Type::HULL:
		{
			Triangle triangle = ((Hull*)volume)->getTriangleInWorldCoord(p_TriangleIndex);
			triangle.uniformScale(100.f);
			return triangle;
		}
			
	case BoundingVolume::Type::OBB:
		{
			XMFLOAT3 triangleIndex = m_BoxTriangleIndex.at(p_TriangleIndex);
			Triangle triangle = Triangle(Vector4(((OBB*)volume)->getCornerWorldCoordAt((int)triangleIndex.x)) * 100.f,
											Vector4(((OBB*)volume)->getCornerWorldCoordAt((int)triangleIndex.y)) * 100.f,
											Vector4(((OBB*)volume)->getCornerWorldCoordAt((int)triangleIndex.z)) * 100.f);
			return triangle;
		}
	case BoundingVolume::Type::SPHERE:
		{
			Triangle triangle = Triangle(Vector4(m_sphereBoundingVolume.at(p_TriangleIndex * 3).m_Postition    ),
											Vector4(m_sphereBoundingVolume.at(p_TriangleIndex * 3 + 1).m_Postition),
											Vector4(m_sphereBoundingVolume.at(p_TriangleIndex * 3 + 2).m_Postition));
			triangle.uniformScale(((Sphere*)volume)->getRadius());
			triangle.translate(volume->getPosition());
			triangle.uniformScale(100.f);
			return triangle;
		}
	default:
		break;
	}

	return trig;
}
unsigned int Physics::getNrOfTrianglesFromBody(unsigned int p_BodyHandle, int p_BoundingVolume)
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to get the number of triangles from non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	BoundingVolume *volume = body->getVolume(p_BoundingVolume);

	switch (volume->getType())
	{
	case BoundingVolume::Type::AABBOX:
	case BoundingVolume::Type::OBB:
		return m_BoxTriangleIndex.size();
	case BoundingVolume::Type::HULL:
		return ((Hull*)volume)->getTriangleListSize();
	case BoundingVolume::Type::SPHERE:
		{
			if(m_LoadBVSphereTemplateOnce)
			{
				m_LoadBVSphereTemplateOnce = false;
				m_BVLoader.loadBinaryFile("assets/LightModels/CB_Sphere.txc");

				m_sphereBoundingVolume = m_BVLoader.getBoundingVolumes();
				m_BVLoader.clear();
			}
			return m_sphereBoundingVolume.size() / 3;
		}
	default:
		break;
	}

	throw PhysicsException("Error! Trying to get the number of triangles from a non existing volume type!", __LINE__, __FILE__);
}

void Physics::setRotation(BodyHandle p_Body, XMMATRIX& p_Rotation)
{
	Body* body = findBody(p_Body);
	if(!body)
		throw PhysicsException("Error! Trying to set rotation on non existing body! BodyHandle =" + std::to_string(p_Body), __LINE__, __FILE__);

	body->setRotation(p_Rotation);
}

unsigned int Physics::getNrOfVolumesInBody(BodyHandle p_BodyHandle)
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to get the number of volumes from non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	return body->getVolumeListSize();
}

Vector3 Physics::getBodyOrientation(BodyHandle p_BodyHandle)
{
	Body* body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to get orientation on non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	BoundingVolume *volume = body->getVolume();

	switch (volume->getType())
	{
	case BoundingVolume::Type::OBB:
		return ((OBB*)volume)->getOrientation();
	case BoundingVolume::Type::AABBOX:
		return Vector3(0.f, 0.f, 0.f);
	case BoundingVolume::Type::SPHERE:
		return Vector3(0.f, 0.f, 0.f);
	case BoundingVolume::Type::HULL:
		return Vector3(0.f, 0.f, 0.f);
	default:
		break;
	}

	throw PhysicsException("Error! Trying to get orientation non existing volume! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);
}

void Physics::resetForceOnBody(BodyHandle p_BodyHandle)
{
	Body *body = findBody(p_BodyHandle);
	if(!body)
		throw PhysicsException("Error! Trying to reset force on non existing body! BodyHandle =" + std::to_string(p_BodyHandle), __LINE__, __FILE__);

	body->resetForce();
}

void Physics::setBodyForceCollisionNormal(BodyHandle p_Body, BodyHandle p_BodyVictim, bool p_Bool)
{
	Body *body = findBody(p_Body);
	Body *bodyVictim = findBody(p_BodyVictim);
	if(!body || ! bodyVictim)
		throw PhysicsException("Error! Trying to force collision normal on non existing body!", __LINE__, __FILE__);


	HitData hit = Collision::boundingVolumeVsBoundingVolume(*body->getVolume(0), *bodyVictim->getVolume(0));
	if(hit.intersect)
	{
		if(!m_IsServer)
		{
			if(body->getCollisionResponse(0) && bodyVictim->getCollisionResponse(0))
			{
				XMVECTOR temp;		// m
				XMFLOAT4 tempPos;	// m

				XMFLOAT4 vel = body->getVelocity();

				XMVECTOR posNorm = Vector4ToXMVECTOR(&hit.colNorm);

				vel.y = 0.f;
				body->setVelocity(vel);

				if(p_Bool)
				{
					posNorm = XMVectorSet(0.f, 1.f, 0.f, 0.f);
				}

				temp = XMLoadFloat4(&body->getPosition()) + posNorm * hit.colLength;
				XMStoreFloat4(&tempPos, temp);

				body->setPosition(tempPos);
			}
		}
	}
}

bool Physics::validBody(BodyHandle p_BodyHandle)
{
	Body *b = findBody(p_BodyHandle);
	if(!b)
		return false;
	else
		return true;
}

bool Physics::isCameraPlayerCollision(Body const &p_Collider, Body const &p_Victim)
{
	if(p_Collider.getGravity() > 0.f && p_Victim.getGravity() > 0.f)
		return false;

	if(!p_Collider.getIsImmovable() && p_Collider.getGravity() == 0.f && !p_Victim.getIsImmovable())
		return true;

	if(!p_Victim.getIsImmovable() && p_Victim.getGravity() == 0.f)
		return true;

	return false;
}