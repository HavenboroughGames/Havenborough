#pragma once
#include "IPhysics.h"
#include "Body.h"
#include "BVLoader.h"
#include "Octree.h"

#include <map>
#include <set>

class Physics : public IPhysics
{
public:
private:
	float m_GlobalGravity;
	float m_Timestep;
	float m_LeftOverTime;
	std::vector<HitData> m_HitDatas;
	BVLoader m_BVLoader;
	bool m_LoadBVSphereTemplateOnce;
	std::vector<std::pair<std::string, std::vector<BVLoader::BoundingVolume>>> m_TemplateBVList;
	std::vector<BVLoader::BoundingVolume> m_sphereBoundingVolume;
	bool m_IsServer;
	std::vector<DirectX::XMFLOAT3> m_BoxTriangleIndex;

	std::map<BodyHandle, Body> m_Bodies;
	Octree m_Octree;
	std::set<BodyHandle> m_PotentialIntersections;
	std::set<BodyHandle> m_MovableBodies;

public:
	Physics();
	~Physics();

	void initialize(bool p_IsServer, float p_Timestep) override;

	void update(float p_DeltaTime, unsigned p_MaxSteps) override;
	void applyForce(BodyHandle p_Body, Vector3 p_Force) override;
	void applyImpulse(BodyHandle p_Body, Vector3 p_Impulse) override;

	BodyHandle createSphere(float p_Mass, bool p_IsImmovable, Vector3 p_Position, float p_Radius) override;
	BodyHandle createAABB(float p_Mass, bool p_IsImmovable, Vector3 p_CenterPos, Vector3 p_Extents, bool p_IsEdge);
	BodyHandle createOBB(float p_Mass, bool p_IsImmovable, Vector3 p_CenterPos, Vector3 p_Extents, bool p_IsEdge) override;

	void addSphereToBody(BodyHandle p_BodyHandle, Vector3 p_Position, float p_Radius) override;
	void addOBBToBody(BodyHandle p_BodyHandle, Vector3 p_CenterPos, Vector3 p_Extents) override;

	BodyHandle createBVInstance(const char* p_VolumeID) override;
	bool createBV(const char* m_ModelID, const char* m_FilePath) override;

	bool releaseBV(const char* p_ModelID) override; 
	void releaseBody(BodyHandle p_Body) override;
	void releaseAllBoundingVolumes(void) override;

	void setGlobalGravity(float p_Gravity) override;
	void setBodyGravity(BodyHandle p_Body, float p_Gravity) override;
	bool getBodyInAir(BodyHandle p_Body) override;
	bool getBodyOnSomething(BodyHandle p_Body) override;

	HitData getHitDataAt(unsigned int p_Index) override;
	unsigned int getHitDataSize() override;

	bool getBodyLanded(BodyHandle p_Body) override;

	void setBodyCollisionResponse(BodyHandle p_Body, bool p_State) override;
	void setBodyVolumeCollisionResponse(BodyHandle p_Body, int volume, bool p_State) override;
	Vector3 getBodyPosition(BodyHandle p_Body) override;

	Vector3 getBodySize(BodyHandle p_Body) override;
	float getSurroundingSphereRadius(BodyHandle p_Body) override;

	void setBodyPosition(BodyHandle p_Body, Vector3 p_Position) override;
	void setBodyVolumePosition(BodyHandle p_Body, unsigned p_Volume, Vector3 p_Position) override;

	void setBodyVelocity(BodyHandle p_Body, Vector3 p_Velocity) override;
	Vector3 getBodyVelocity(BodyHandle p_Body) override;

	void setBodyRotation(BodyHandle p_Body, Vector3 p_Rotation) override;
	void setBodyRotationMatrix(BodyHandle p_Body, DirectX::XMFLOAT4X4 p_Rotation) override;
	void setBodyScale(BodyHandle p_BodyHandle, Vector3 p_Scale) override;

	void setLogFunction(clientLogCallback_t p_LogCallback) override;

	Triangle getTriangleFromBody(unsigned int p_BodyHandle, unsigned int p_TriangleIndex, int p_BoundingVolume) override;
	unsigned int getNrOfTrianglesFromBody(unsigned int p_BodyHandle, int p_BoundingVolume) override;
	unsigned int getNrOfVolumesInBody(BodyHandle p_BodyHandle);
	Vector3 getBodyOrientation(BodyHandle p_BodyHandle) override;
	void resetForceOnBody(BodyHandle p_BodyHandle) override;	

	void setBodyForceCollisionNormal(BodyHandle p_Body, BodyHandle p_BodyVictim, bool p_Bool) override;
	bool validBody(BodyHandle p_BodyHandle);

	float getTimestep() const override;

	BodyHandle rayCast(const DirectX::XMFLOAT4 &p_RayDirection, const DirectX::XMFLOAT4 &p_RayOrigin) override;

private:
	Body* findBody(BodyHandle p_Body);
	
	BodyHandle createBody(float p_Mass, BoundingVolume* p_BoundingVolume, bool p_IsImmovable, bool p_IsEdge);

	BoundingVolume* getVolume(BodyHandle p_Body);

	void fillTriangleIndexList();

	void setRotation(BodyHandle p_Body, DirectX::XMMATRIX& p_Rotation);

	void singleCollisionCheck(Body& p_Collider, Body& p_Victim, bool& p_IsOnGround);
	void handleCollision(HitData p_Hit, Body& p_Collider, int p_ColliderVolumeId, Body& p_Victim, int p_VictimVolumeID, bool &p_IsOnGround);

	bool isCameraPlayerCollision(Body const &p_Collider, Body const &p_Victim);
};

