#include <boost\test\unit_test.hpp>
#include "..\..\Physics\Source\Collision.h"
#include "..\..\Physics\Source\PhysicsExceptions.h"

class DummyBoundingVolume : public BoundingVolume
{
	Sphere m_Sphere;
public:
	DummyBoundingVolume() :
		BoundingVolume(&m_Sphere)
	{
		m_Type = Type::NONE;
		m_BodyHandle = 0;
	}
	~DummyBoundingVolume(){}

	void updatePosition(DirectX::XMFLOAT4X4 const &p_Translation) override
	{
		
	}
	
	void setPosition(DirectX::XMVECTOR const &p_Position) override
	{
		
	}

	void scale(DirectX::XMVECTOR const &p_Scale) override
	{
		
	}

	void setRotation(DirectX::XMMATRIX const &p_Rotation) override
	{
		
	}
};


BOOST_AUTO_TEST_SUITE(CollisionTest)

BOOST_AUTO_TEST_CASE(SphereVsSphereHit)
{
	Sphere s1, s2;

	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f));
	s2 = Sphere(100.f, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));

	HitData hitData = Collision::boundingVolumeVsBoundingVolume(s1, s2);

	BOOST_CHECK(hitData.intersect);
	BOOST_CHECK_CLOSE_FRACTION(hitData.colLength, 100.f, 0.001f);
	BOOST_CHECK_SMALL(hitData.colNorm.x, 0.001f);
	BOOST_CHECK_CLOSE_FRACTION(hitData.colNorm.y, 1.f, 0.001f);
	BOOST_CHECK_SMALL(hitData.colNorm.z, 0.001f);
}

BOOST_AUTO_TEST_CASE(SphereVsSphereMiss)
{
	Sphere s1, s2;

	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 10.0f, 0.0f, 0.0f));
	s2 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));

	BOOST_CHECK_EQUAL(Collision::boundingVolumeVsBoundingVolume(s1, s2).intersect, false);
}

BOOST_AUTO_TEST_CASE(AABBVsSphereHitOrigin)
{
	Sphere s1;
	AABB aabb;

	HitData hit = HitData();

	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));

	aabb = AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.f));

	hit = Collision::boundingVolumeVsBoundingVolume(aabb, s1);

	BOOST_CHECK_EQUAL(hit.intersect, true);
	BOOST_CHECK_EQUAL(hit.colNorm.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colNorm.y, 0.f);
	BOOST_CHECK_EQUAL(hit.colNorm.z, 0.f);

	BOOST_CHECK_EQUAL(hit.colLength, 1.f);

	BOOST_CHECK_EQUAL(hit.colPos.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colPos.y, 0.f);
	BOOST_CHECK_EQUAL(hit.colPos.z, 0.f);

	BOOST_CHECK_EQUAL((int)hit.colType, (int)Type::AABBVSSPHERE);
}

BOOST_AUTO_TEST_CASE(AABBVsSphereHit)
{
	Sphere s1;
	AABB aabb;

	HitData hit = HitData();

	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 2.f, 0.0f, 1.0f));

	aabb = AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));

	hit = Collision::boundingVolumeVsBoundingVolume(aabb, s1);

	BOOST_CHECK_EQUAL(hit.intersect, true);
	BOOST_CHECK_EQUAL(hit.colNorm.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colNorm.y, 1.f);
	BOOST_CHECK_EQUAL(hit.colNorm.z, 0.f);

	BOOST_CHECK_EQUAL(hit.colLength, 0.f);

	BOOST_CHECK_EQUAL(hit.colPos.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colPos.y, 100.f);
	BOOST_CHECK_EQUAL(hit.colPos.z, 0.f);

	BOOST_CHECK_EQUAL((int)hit.colType, (int)Type::AABBVSSPHERE);
}

BOOST_AUTO_TEST_CASE(EXTREMECASE_AABBVsSphereHit)
{
	Sphere s1;
	AABB aabb;

	s1 = Sphere(0.1f, DirectX::XMFLOAT4(0.0f, 50.0f, 0.0f, 0.0f));

	aabb = AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(100.f, 100.f, 100.f, 1.f));

	BOOST_CHECK_EQUAL(Collision::boundingVolumeVsBoundingVolume(aabb, s1).intersect, true);
}

BOOST_AUTO_TEST_CASE(EXTREMECASE2_AABBVsSphereHit)
{
	Sphere s1;
	AABB aabb;

	HitData hit = HitData();

	s1 = Sphere(1000.f, DirectX::XMFLOAT4(1001.f, 0.0f, 0.0f, 1.0f));

	aabb = AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 0.f));

	hit = Collision::boundingVolumeVsBoundingVolume(aabb, s1);

	BOOST_CHECK_EQUAL(hit.intersect, true);
	BOOST_CHECK_EQUAL(hit.colNorm.x, 1.f);
	BOOST_CHECK_EQUAL(hit.colNorm.y, 0.f);
	BOOST_CHECK_EQUAL(hit.colNorm.z, 0.f);

	BOOST_CHECK_EQUAL(hit.colLength, 0.f);

	BOOST_CHECK_EQUAL(hit.colPos.x, 100.f);
	BOOST_CHECK_EQUAL(hit.colPos.y, 0.f);
	BOOST_CHECK_EQUAL(hit.colPos.z, 0.f);

	BOOST_CHECK_EQUAL((int)hit.colType, (int)Type::AABBVSSPHERE);
}

BOOST_AUTO_TEST_CASE(AABBVsSphereMiss)
{
	Sphere s1;
	AABB aabb;

	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 10.0f, 0.0f, 0.0f));

	aabb = AABB(DirectX::XMFLOAT4(-1.f, -1.f, -1.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));

	BOOST_CHECK_EQUAL(Collision::boundingVolumeVsBoundingVolume(s1, aabb).intersect, false);
}

BOOST_AUTO_TEST_CASE(AABBvsAABBHit)
{
	AABB aabb, aabbb;

	aabb = AABB(DirectX::XMFLOAT4(-1.f, -1.f, -1.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	aabbb = AABB(DirectX::XMFLOAT4(-1.f, -1.f, -1.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));

	BOOST_CHECK_EQUAL(Collision::boundingVolumeVsBoundingVolume(aabb, aabbb).intersect, true);
}

BOOST_AUTO_TEST_CASE(AABBvsAABBMiss)
{
	AABB aabb, aabbb;

	aabb = AABB(DirectX::XMFLOAT4(-2.f, -2.f, -2.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	aabbb = AABB(DirectX::XMFLOAT4(2.f, 2.f, 2.f, 1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f));

	BOOST_CHECK_EQUAL(Collision::boundingVolumeVsBoundingVolume(aabb, aabbb).intersect, false);
}

BOOST_AUTO_TEST_CASE(BoundingVolumeVsBoundingVolumeHit)
{
	HitData hit = HitData();
	
	Sphere s1, s2;
	AABB aabb1, aabb2;
	
	s1 = Sphere(1.f, DirectX::XMFLOAT4(0.0f, 2.5f, 0.0f, 1.0f));
	s2 = Sphere(2.f, DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

	aabb1 = AABB(DirectX::XMFLOAT4(-1.f, -1.f, -1.f, 1.f), DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	aabb2 = AABB(DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f), DirectX::XMFLOAT4(2.f, 2.f, 2.f, 1.f));

	hit = Collision::boundingVolumeVsBoundingVolume(s1, s2);

	BOOST_CHECK_EQUAL((int)hit.colType, (int)Type::SPHEREVSSPHERE);
	BOOST_CHECK_EQUAL(hit.colNorm.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colNorm.y, 1.f);
	BOOST_CHECK_EQUAL(hit.colNorm.z, 0.f);

	BOOST_CHECK_EQUAL(hit.colPos.x, 0.f);
	BOOST_CHECK_EQUAL(hit.colPos.y, 200.f);
	BOOST_CHECK_EQUAL(hit.colPos.z, 0.f);
	
	BOOST_CHECK_EQUAL(hit.colLength, 0.5f);
}

BOOST_AUTO_TEST_CASE(OBBvsOBBHit)
{
	HitData hd;
	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0.f, 5.f, 0.f);
	DirectX::XMFLOAT4X4 mtrans;
	DirectX::XMStoreFloat4x4(&mtrans, trans);

	OBB obb1 = OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	OBB obb2 = OBB(DirectX::XMFLOAT4(0.f, 5.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
	
	obb2.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);

	obb2.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(!hd.intersect);

	trans = DirectX::XMMatrixTranslation(0.f, -0.5f, 0.f);
	DirectX::XMStoreFloat4x4(&mtrans, trans);

	//A box moving through a box standing still
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::OBBvsOBB(obb1, obb2);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50,"Error on iteration: " + std::to_string(i) + ".\n");
		}
		obb2.updatePosition(mtrans);
	}

	//A rotating box moving through a box standing still.
	obb2 = OBB(DirectX::XMFLOAT4(0.f, 15.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(3.1415926f/4.f);
	obb2.setRotation(rot);
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
		obb2.updatePosition(mtrans);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50, "Error on rotation iteration: " + std::to_string(i) + ".\n");
		}
	}
}

BOOST_AUTO_TEST_CASE(OBBvsAABBHit)
{
	HitData hd;
	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0.f, 5.f, 0.f);
	DirectX::XMFLOAT4X4 mtrans;
	DirectX::XMStoreFloat4x4(&mtrans, trans);

	OBB obb		= OBB(DirectX::XMFLOAT4(0.f, 5.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	AABB aabb	= AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb, aabb);
	BOOST_CHECK(hd.intersect);
	
	obb.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb, aabb);
	BOOST_CHECK(hd.intersect);

	obb.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb, aabb);
	BOOST_CHECK(!hd.intersect);

	trans = DirectX::XMMatrixTranslation(0.f, -0.5f, 0.f);
	DirectX::XMStoreFloat4x4(&mtrans, trans);
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::boundingVolumeVsBoundingVolume(aabb, obb);
		obb.updatePosition(mtrans);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50,"Error on iteration: " + std::to_string(i) + ".\n");
		}
		
	}

	//Rotation
	obb = OBB(DirectX::XMFLOAT4(0.f, 15.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(3.1415926f/4.f);
	obb.setRotation(rot);
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::boundingVolumeVsBoundingVolume(obb, aabb);
		obb.updatePosition(mtrans);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50, "Error on rotation iteration LOLZ!: " + std::to_string(i) + ".\n");
		}
	}
}

BOOST_AUTO_TEST_CASE(OBBvsSphereHit)
{
	HitData hd;
	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0.f, 5.f, 0.f);
	DirectX::XMFLOAT4X4 mtrans;
	DirectX::XMStoreFloat4x4(&mtrans, trans);

	OBB obb		= OBB(DirectX::XMFLOAT4(0.f, 5.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	Sphere s	= Sphere(5.f, DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb, s);
	BOOST_CHECK(hd.intersect);
	
	obb.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb, s);
	BOOST_CHECK(hd.intersect);

	obb.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb, s);
	BOOST_CHECK(!hd.intersect);

	trans = DirectX::XMMatrixTranslation(0.f, -0.5f, 0.f);
	DirectX::XMStoreFloat4x4(&mtrans, trans);
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::boundingVolumeVsBoundingVolume(obb, s);
		obb.updatePosition(mtrans);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50,"OBB Collision Error on iteration: " + std::to_string(i) + ".\n");
		}
		
	}

	//Rotation
	obb = OBB(DirectX::XMFLOAT4(0.f, 15.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 1.f));
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(3.1415926f/4.f);
	obb.setRotation(rot);
	for(int i = 0; i < 100; i++)
	{
		hd = Collision::boundingVolumeVsBoundingVolume(s, obb);
		obb.updatePosition(mtrans);
		if(hd.intersect == true)
		{
			BOOST_CHECK_MESSAGE(i >= 10 && i <= 50, "OBB Collision Error on rotation iteration: " + std::to_string(i) + ".\n");
		}
	}
}

BOOST_AUTO_TEST_CASE(OBBInsideOBB)
{
	HitData hd;

	OBB obb1 = OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f));
	OBB obb2 = OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(1000.f, 1000.f, 1000.f, 1000.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
}

BOOST_AUTO_TEST_CASE(OBBInsideSphereAndViceVersa)
{
	HitData hd;

	OBB obb		= OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f));
	Sphere s	= Sphere(1000.f, DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	//OBB inside sphere
	hd = Collision::boundingVolumeVsBoundingVolume(obb, s);
	BOOST_CHECK(hd.intersect);

	obb.setExtent(DirectX::XMVectorSet(1000.f, 1000.f, 1000.f, 1.f));
	s.setRadius(0.1f);
	///sphere inside OBB
	hd = Collision::boundingVolumeVsBoundingVolume(s, obb);
	BOOST_CHECK(hd.intersect);
}

BOOST_AUTO_TEST_CASE(OBBInsideAABBAndViceVersa)
{
	HitData hd;

	OBB obb		= OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f));
	AABB aabb	= AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(1000.f, 1000.f, 1000.f, 1.f));

	///OBB inside AABB
	hd = Collision::boundingVolumeVsBoundingVolume(obb, aabb);
	BOOST_CHECK(hd.intersect);

	obb.setExtent(DirectX::XMVectorSet(1000.f, 1000.f, 1000.f, 1.f));
	aabb = AABB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f));
	///AABB inside OBB
	hd = Collision::boundingVolumeVsBoundingVolume(aabb, obb);
	BOOST_CHECK(hd.intersect);
}

BOOST_AUTO_TEST_CASE(HullvsSphereHit)
{	
		HitData hd;
		std::vector<Triangle> triangles;
		float size = 1.f;
		triangles.push_back(Triangle(Vector4( -size,  -size, -size, 1.f), Vector4(-size, size, -size, 1.f), Vector4(size,	size, -size, 1.f)));
		triangles.push_back(Triangle(Vector4( -size,  -size, -size, 1.f), Vector4( size, size, -size, 1.f), Vector4(size, -size, -size, 1.f)));

		Hull h  = Hull(triangles);
		Sphere s = Sphere(2.f, DirectX::XMFLOAT4(2.f, 0.f, 0.f, 1.f));
		
		hd = Collision::boundingVolumeVsBoundingVolume(h, s);

		BOOST_CHECK(hd.intersect);

		s.setPosition(DirectX::XMVectorSet(2.2f, 0.f, 0.f, 1.f));

		hd = Collision::boundingVolumeVsBoundingVolume(h, s);

		BOOST_CHECK(hd.intersect);

		s.setPosition(DirectX::XMVectorSet(3.1f, 0.f, 0.f, 1.f));

		hd = Collision::boundingVolumeVsBoundingVolume(h, s);

		BOOST_CHECK(!hd.intersect);

		DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(1.f, 0.f, 0.f);
		DirectX::XMFLOAT4X4 mtrans;
		DirectX::XMStoreFloat4x4(&mtrans, trans);
		h.updatePosition(mtrans);

		hd = Collision::boundingVolumeVsBoundingVolume(h, s);
		BOOST_CHECK(hd.intersect);
}

BOOST_AUTO_TEST_CASE(HullInBigSphere)
{
	
	HitData hd;
	std::vector<Triangle> triangles;
	float size = 1.f;
	triangles.push_back(Triangle(Vector4( -size,  -size, -size, 1.f), Vector4(-size, size, -size, 1.f), Vector4(size,	size, -size, 1.f)));
	triangles.push_back(Triangle(Vector4( -size,  -size, -size, 1.f), Vector4( size, size, -size, 1.f), Vector4(size, -size, -size, 1.f)));

	Hull h  = Hull(triangles);
	Sphere s = Sphere(2000.f, DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f));

	hd = Collision::boundingVolumeVsBoundingVolume(s, h);

	BOOST_CHECK(hd.intersect);
}

BOOST_AUTO_TEST_CASE(OBBCollisionVectorTest)
{
	HitData hd;
	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(0.f, 5.f, 0.f);
	DirectX::XMFLOAT4X4 mtrans;
	DirectX::XMStoreFloat4x4(&mtrans, trans);

	OBB obb1 = OBB(DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 0.f));
	OBB obb2 = OBB(DirectX::XMFLOAT4(0.f, 5.f, 0.f, 1.f), DirectX::XMFLOAT4(5.f, 5.f, 5.f, 0.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
	BOOST_CHECK_CLOSE_FRACTION(hd.colLength, 5.f, 0.01f);
	BOOST_CHECK_SMALL(hd.colNorm.x, 0.01f);
	BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.y, -1.f, 0.01f);
	BOOST_CHECK_SMALL(hd.colNorm.z, 0.01f);
	
	obb2.updatePosition(mtrans);
	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
	BOOST_CHECK_SMALL(hd.colLength, 0.01f);
	BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
	BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.y, -1.f, 0.0001f);
	BOOST_CHECK_SMALL(hd.colNorm.z, 0.0001f);

	obb1 = OBB(DirectX::XMFLOAT4(-10.f, 2.f, 40.f, 1.f), DirectX::XMFLOAT4(0.5f, 0.9f, 0.5f, 0.f));
	obb1.setRotation(DirectX::XMMatrixRotationRollPitchYaw(0.f, 1.f, 0.f));
	obb2 = OBB(DirectX::XMFLOAT4(-10.f, 0.f, 40.f, 1.f), DirectX::XMFLOAT4(5.f, 1.2f, 5.f, 0.f));
	obb2.setRotation(DirectX::XMMatrixRotationRollPitchYaw(0.f, 1.f, 0.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
	BOOST_CHECK_CLOSE_FRACTION(hd.colLength, 0.1f, 0.001f);
	BOOST_CHECK_SMALL(hd.colNorm.x, 0.001f);
	BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.y, 1.f, 0.001f);
	BOOST_CHECK_SMALL(hd.colNorm.z, 0.001f);

	DirectX::XMFLOAT4 fCorner(5.f, 0.f, 5.f, 0.f);
	DirectX::XMVECTOR corner = DirectX::XMLoadFloat4(&fCorner);
	corner = DirectX::XMVector4Transform(corner, DirectX::XMMatrixRotationRollPitchYaw(0.f, 1.f, 0.f));


	obb1 = OBB(DirectX::XMFLOAT4(-10.f + corner.m128_f32[0] + 0.4f, 0.f, 40.f + corner.m128_f32[2], 1.f), DirectX::XMFLOAT4(0.5f, 0.9f, 0.5f, 0.f));

	hd = Collision::boundingVolumeVsBoundingVolume(obb1, obb2);
	BOOST_CHECK(hd.intersect);
	BOOST_CHECK_CLOSE_FRACTION(hd.colLength, 0.1f, 0.001f);
	BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.x, 1.f, 0.001f);
	BOOST_CHECK_SMALL(hd.colNorm.y, 0.001f);
	BOOST_CHECK_SMALL(hd.colNorm.z, 0.001f);
}

BOOST_AUTO_TEST_CASE(HullvsOBB)
{	
		HitData hd;
		std::vector<Triangle> triangles;
		float size = 2.f;

		triangles.push_back(Triangle(Vector4( -size, -size, -size, 1.f), Vector4(-size, size, -size, 1.f), Vector4(size,  size, -size, 1.f)));
		triangles.push_back(Triangle(Vector4( -2.f, -2.f, -2.f, 1.f), Vector4( 2.f, 2.f, -2.f, 1.f), Vector4(2.f, -2.f, -2.f, 1.f)));

		Hull h  = Hull(triangles);
		OBB obb = OBB(DirectX::XMFLOAT4(-0.5f, 0.0f, -2.1f, 1.f), DirectX::XMFLOAT4(.5f, .5f, .5f, 0.f));	

		hd = Collision::boundingVolumeVsBoundingVolume(obb, h);
		BOOST_CHECK(hd.intersect);
		BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
		BOOST_CHECK_SMALL(hd.colNorm.y, 0.0001f);
		BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.z, -1.f ,0.0001f);
	
		obb.setPosition(DirectX::XMVectorSet(-0.5f, 0.0f, -2.2f, 1.f));
		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(hd.intersect);
		BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
		BOOST_CHECK_SMALL(hd.colNorm.y, 0.0001f);
		BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.z, -1.f ,0.0001f);

		obb.setPosition(DirectX::XMVectorSet(-0.5f, 0.0f, -2.3f, 1.f));
		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(hd.intersect);
		BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
		BOOST_CHECK_SMALL(hd.colNorm.y, 0.0001f);
		BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.z, -1.f ,0.0001f);

		obb.setPosition(DirectX::XMVectorSet(-0.5f, 0.0f, -2.4f, 1.f));
		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(hd.intersect);
		BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
		BOOST_CHECK_SMALL(hd.colNorm.y, 0.0001f);
		BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.z, -1.f ,0.0001f);

		obb.setPosition(DirectX::XMVectorSet(-0.5f, 0.0f, -2.5f, 1.f));
		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(!hd.intersect);

		obb.setPosition(DirectX::XMVectorSet(-0.5f, 0.0f, -2.6f, 1.f));
		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(!hd.intersect);

		triangles.clear();
		triangles.push_back(Triangle(Vector4( -2.f, 2.f, -2.f, 1.f), Vector4(-2.f, 2.f, 2.f, 1.f), Vector4( 2.f, 2.f, 2.f, 1.f)));
		h  = Hull(triangles);
		obb = OBB(DirectX::XMFLOAT4(-0.5f, 2.1f, 0.0f, 1.f), DirectX::XMFLOAT4(.5f, .5f, .5f, 0.f));

		hd = Collision::boundingVolumeVsBoundingVolume(h, obb);
		BOOST_CHECK(hd.intersect);
		BOOST_CHECK_SMALL(hd.colNorm.x, 0.0001f);
		BOOST_CHECK_CLOSE_FRACTION(hd.colNorm.y, 1.f ,0.0001f);
		BOOST_CHECK_SMALL(hd.colNorm.z, 0.0001f);
		
}

BOOST_AUTO_TEST_CASE(CollisionTypeNotFound)
{
	DummyBoundingVolume fakeVolume = DummyBoundingVolume();

	OBB obb = OBB(DirectX::XMFLOAT4(-0.5f, 2.1f, 0.0f, 1.f), DirectX::XMFLOAT4(.5f, .5f, .5f, 0.f));
	Sphere s = Sphere(5.f, DirectX::XMFLOAT4(.5f, .5f, .5f, 0.f));
	AABB aabb = AABB(DirectX::XMFLOAT4(-0.5f, 2.1f, 0.0f, 1.f), DirectX::XMFLOAT4(.5f, .5f, .5f, 0.f));
	std::vector<Triangle> t;
	Hull h = Hull(t);

	HitData hd;
	BOOST_CHECK_THROW(hd = Collision::boundingVolumeVsBoundingVolume(fakeVolume, fakeVolume), CollisionException);

	BOOST_CHECK_THROW(hd = Collision::boundingVolumeVsSphere(fakeVolume, s), CollisionException);

	BOOST_CHECK_THROW(hd = Collision::boundingVolumeVsAABB(fakeVolume, aabb), CollisionException);

	BOOST_CHECK_THROW(hd = Collision::boundingVolumeVsOBB(fakeVolume, obb), CollisionException);

	BOOST_CHECK_THROW(hd = Collision::boundingVolumeVsHull(fakeVolume, h), CollisionException);

}

BOOST_AUTO_TEST_SUITE_END()