#include "Collision.h"
#include "PhysicsExceptions.h"
#include "PhysicsLogger.h"
#define EPSILON XMVectorGetX(g_XMEpsilon)
using namespace DirectX;



HitData Collision::boundingVolumeVsBoundingVolume(BoundingVolume const &p_Volume1, BoundingVolume const &p_Volume2)
{
	if(p_Volume1.getBodyHandle() == p_Volume2.getBodyHandle())
		if(p_Volume1.getBodyHandle() != 0)
				return HitData();

	BoundingVolume::Type type = p_Volume2.getType();

	switch(type)
	{		
	case BoundingVolume::Type::AABBOX:
		return boundingVolumeVsAABB(p_Volume1, (AABB&)p_Volume2);
	case BoundingVolume::Type::SPHERE:
		return boundingVolumeVsSphere(p_Volume1, (Sphere&) p_Volume2);
	case BoundingVolume::Type::OBB:
		return boundingVolumeVsOBB(p_Volume1, (OBB&)p_Volume2);
	case BoundingVolume::Type::HULL:
		return boundingVolumeVsHull(p_Volume1, (Hull&)p_Volume2);
	default:
		throw CollisionException("Collision error! Bounding volume type does not exist!", __LINE__, __FILE__);
	}
}

HitData Collision::boundingVolumeVsSphere(BoundingVolume const &p_Volume, Sphere const &p_Sphere)
{
	BoundingVolume::Type type = p_Volume.getType();

	switch(type)
	{
	case BoundingVolume::Type::AABBOX:
		return AABBvsSphere((AABB&)p_Volume, p_Sphere);
	case BoundingVolume::Type::SPHERE:
		return sphereVsSphere((Sphere&)p_Volume, p_Sphere);
	case BoundingVolume::Type::OBB:
		return OBBvsSphere((OBB&)p_Volume, p_Sphere);
	case BoundingVolume::Type::HULL:
		return HullVsSphere((Hull&)p_Volume, p_Sphere);
	default:
		throw CollisionException("Collision error! Bounding volume type does not exist!", __LINE__, __FILE__);
	}
}

HitData Collision::boundingVolumeVsAABB(BoundingVolume const &p_Volume, AABB const &p_AABB)
{
	BoundingVolume::Type type = p_Volume.getType();
	switch(type)
	{
	case BoundingVolume::Type::AABBOX:
		return AABBvsAABB((AABB&)p_Volume, p_AABB);
	case BoundingVolume::Type::SPHERE:
		return AABBvsSphere(p_AABB, (Sphere&)p_Volume);
	case BoundingVolume::Type::OBB:
		return OBBvsAABB((OBB&)p_Volume, p_AABB);
	default:
		throw CollisionException("Collision error! Bounding volume type does not exist!", __LINE__, __FILE__);
	}
}

HitData Collision::boundingVolumeVsOBB(BoundingVolume const &p_Volume, OBB const &p_OBB)
{
	BoundingVolume::Type type = p_Volume.getType();
	switch(type)
	{
	case BoundingVolume::Type::AABBOX:
		return OBBvsAABB(p_OBB, (AABB&)p_Volume);
	case BoundingVolume::Type::SPHERE:
		return OBBvsSphere(p_OBB, (Sphere&)p_Volume);
	case BoundingVolume::Type::OBB:
		return OBBvsOBB((OBB&)p_Volume, p_OBB);
	case BoundingVolume::Type::HULL:
		return OBBVsHull(p_OBB, (Hull&)p_Volume);
	default:
		throw CollisionException("Collision error! Bounding volume type does not exist!", __LINE__, __FILE__);
	}
}

HitData Collision::boundingVolumeVsHull(BoundingVolume const &p_Volume, Hull const &p_Hull)
{
	BoundingVolume::Type type = p_Volume.getType();
	switch(type)
	{
	case BoundingVolume::Type::SPHERE:
			return HullVsSphere(p_Hull, (Sphere&)p_Volume);
	case BoundingVolume::Type::OBB:
		return OBBVsHull((OBB&)p_Volume, p_Hull);
	default:
		throw CollisionException("Collision error! Bounding volume type does not exist!", __LINE__, __FILE__);
	}
}

bool Collision::surroundingSphereVsSphere(Sphere const &p_Sphere1, Sphere const &p_Sphere2 )
{
	XMFLOAT4 s1Pos = p_Sphere1.getPosition();
	XMFLOAT4 s2Pos = p_Sphere2.getPosition();
	XMVECTOR CDiff = XMVectorSet(s2Pos.x - s1Pos.x, s2Pos.y - s1Pos.y, s2Pos.z - s1Pos.z, s2Pos.w - s1Pos.w);
	
	float c = XMVector3LengthSq(CDiff).m128_f32[0]; // m^2
	float rSum = p_Sphere2.getRadius() + p_Sphere1.getRadius();	// m
    float rSumSqr = rSum*rSum;	// m^2

	//Find out if the sphere centers are separated with more distance than the radiuses.
    if(c <= rSumSqr)
	{
		return true;
	}

	return false;
}

HitData Collision::sphereVsSphere(Sphere const &p_Sphere1, Sphere const &p_Sphere2 )
{
	HitData hit;

	XMFLOAT4 s1Pos = p_Sphere1.getPosition();
	XMFLOAT4 s2Pos = p_Sphere2.getPosition();
	XMVECTOR CDiff = XMVectorSet(s2Pos.x - s1Pos.x, s2Pos.y - s1Pos.y, s2Pos.z - s1Pos.z, s2Pos.w - s1Pos.w);
	
	float c = XMVectorGetX(XMVector3LengthSq(CDiff)); // m^2
	float rSum = p_Sphere2.getRadius() + p_Sphere1.getRadius();	// m
    float rSumSqr = rSum*rSum;	// m^2

	//Find out if the sphere centers are separated with more distance than the radiuses.
    if(c <= rSumSqr)
	{
		hit.intersect = true;

		XMFLOAT4 position;	// m
		
		XMVECTOR normalized = XMVector4Normalize(XMVectorSet(s1Pos.x - s2Pos.x, s1Pos.y - s2Pos.y, s1Pos.z - s2Pos.z, s1Pos.w - s2Pos.w));
		XMVECTOR hitPos = normalized  * p_Sphere2.getRadius();	// m

		hit.colPos = hitPos * 100.f;
		hit.colNorm = normalized;

		hit.colLength = (rSum - sqrtf(c));

		hit.colType = Type::SPHEREVSSPHERE;
	}

	return hit;
}

HitData Collision::AABBvsAABB(AABB const &p_AABB1, AABB const &p_AABB2 )
{	
	if(!surroundingSphereVsSphere(p_AABB1.getSphere(), p_AABB2.getSphere()))
		return HitData();

	HitData hit = HitData();
	XMFLOAT4 max1 = p_AABB1.getMax();	// m
	XMFLOAT4 min1 = p_AABB1.getMin();	// m
	XMFLOAT4 max2 = p_AABB2.getMax();	// m
	XMFLOAT4 min2 = p_AABB2.getMin();	// m

	//Test if the boxes are separated in any axis.
	if ( min1.x > max2.x || min2.x > max1.x )
		return hit;
	if ( min1.y > max2.y || min2.y > max1.y )
		return hit;
	if ( min1.z > max2.z || min2.z > max1.z )
		return hit;
		
	hit.intersect = true;
	hit.colType = Type::AABBVSAABB;

	return hit;
}

HitData Collision::AABBvsSphere(AABB const &p_AABB, Sphere const &p_Sphere)
{
	if(!surroundingSphereVsSphere(p_AABB.getSphere(), p_Sphere))
		return HitData();

	HitData hit;

	//Check to see if the sphere overlaps the AABB
	//const bool AABBOverlapsSphere ( const AABB& B, const SCALAR r, VECTOR& C )
	float s = 0; // m
	float d = 0; // m^2

	//find the square of the distance
	//from the sphere to the box
	XMFLOAT4 spherePos = p_Sphere.getPosition();	// m
	XMFLOAT3 dist;	// m

	//if the sphere is outside of the box, find the corner closest to the sphere center in each axis.
	//else special case for when the sphere center is inside that axis slab.

	XMFLOAT4 bMin = p_AABB.getMin();	// m
	XMFLOAT4 bMax = p_AABB.getMax();	// m

	// x
	if( spherePos.x <= bMin.x )
	{
		dist.x = bMin.x;
	}
	else if( spherePos.x > bMax.x )
	{
		dist.x = bMax.x;
	}
	else
		dist.x = spherePos.x;

	s = spherePos.x - dist.x;
	d += s*s;

	// y
	if( spherePos.y <= bMin.y )
	{
		dist.y = bMin.y;
	}
	else if( spherePos.y > bMax.y )
	{
		dist.y = bMax.y;
	}
	else
		dist.y = spherePos.y;

	s = spherePos.y - dist.y;
	d += s*s;

	// z
	if( spherePos.z <= bMin.z )
	{
		dist.z = bMin.z;
	}
	else if( spherePos.z > bMax.z )
	{
		dist.z = bMax.z;
	}
	else
		dist.z = spherePos.z;

	s = spherePos.z - dist.z;
	d += s*s;

	if(d <= p_Sphere.getSqrRadius())
	{
		hit.intersect = true;
		hit.colPos.x = dist.x * 100.f;
		hit.colPos.y = dist.y * 100.f;
		hit.colPos.z = dist.z * 100.f;
		hit.colPos.w = 1.f;

		//XMFLOAT4 colPos(dist.x, dist.y, dist.z, 1.f);
		XMVECTOR colPos = XMLoadFloat3(&dist);
		colPos = XMVectorSetW(colPos, 1.f);
		XMVECTOR tempNorm = XMVector4Normalize(XMLoadFloat4(&spherePos) - colPos);

		hit.colNorm = tempNorm;
		hit.colLength = (p_Sphere.getRadius() - sqrtf(d));

		hit.colType = Type::AABBVSSPHERE;
	}

	return hit;
}

bool Collision::AABBvsSphereIntersect(XMFLOAT4 p_Min, XMFLOAT4 p_Max, Sphere const &p_Sphere)
{
	//Check to see if the sphere overlaps the AABB
	//const bool AABBOverlapsSphere ( const AABB& B, const SCALAR r, VECTOR& C )
	float s = 0; // m
	float d = 0; // m^2

	//find the square of the distance
	//from the sphere to the box
	XMFLOAT4 spherePos = p_Sphere.getPosition();	// m
	XMFLOAT3 dist;	// m

	//if the sphere is outside of the box, find the corner closest to the sphere center in each axis.
	//else special case for when the sphere center is inside that axis slab.


	// x
	if( spherePos.x <= p_Min.x )
	{
		dist.x = p_Min.x;
	}
	else if( spherePos.x > p_Max.x )
	{
		dist.x = p_Max.x;
	}
	else
		dist.x = spherePos.x;

	s = spherePos.x - dist.x;
	d += s*s;

	// y
	if( spherePos.y <= p_Min.y )
	{
		dist.y = p_Min.y;
	}
	else if( spherePos.y > p_Max.y )
	{
		dist.y = p_Max.y;
	}
	else
		dist.y = spherePos.y;

	s = spherePos.y - dist.y;
	d += s*s;

	// z
	if( spherePos.z <= p_Min.z )
	{
		dist.z = p_Min.z;
	}
	else if( spherePos.z > p_Max.z )
	{
		dist.z = p_Max.z;
	}
	else
		dist.z = spherePos.z;

	s = spherePos.z - dist.z;
	d += s*s;

	if(d <= p_Sphere.getSqrRadius())
	{
		return true;
	}
	return false;
}

bool Collision::AABBInsideSphere(XMFLOAT4 p_Min, XMFLOAT4 p_Max, Sphere const &p_Sphere)
{
	XMFLOAT4 spherePos = p_Sphere.getPosition();
	float radius = p_Sphere.getRadius();

	//xyz
	XMVECTOR v = XMVectorSet(p_Min.x - spherePos.x, p_Min.y - spherePos.y, p_Min.z - spherePos.z, 0.f);
	float l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//Xyz
	v = XMVectorSet(p_Max.x - spherePos.x, p_Min.y - spherePos.y, p_Min.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;
	
	//xYz
	v = XMVectorSet(p_Min.x - spherePos.x, p_Max.y - spherePos.y, p_Min.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//XYz
	v = XMVectorSet(p_Max.x - spherePos.x, p_Max.y - spherePos.y, p_Min.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//xyZ
	v = XMVectorSet(p_Min.x - spherePos.x, p_Min.y - spherePos.y, p_Max.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//XyZ
	v = XMVectorSet(p_Max.x - spherePos.x, p_Min.y - spherePos.y, p_Max.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//xYZ
	v = XMVectorSet(p_Min.x - spherePos.x, p_Max.y - spherePos.y, p_Max.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	//XYZ
	v = XMVectorSet(p_Max.x - spherePos.x, p_Max.y - spherePos.y, p_Max.z - spherePos.z, 0.f);
	l = XMVector3Length(v).m128_f32[0];
	if(l > radius)
		return false;

	return true;
}

bool Collision::SphereInsideAABB(XMFLOAT4 p_Min, XMFLOAT4 p_Max, Sphere const &p_Sphere)
{
	XMFLOAT4 spherePos = p_Sphere.getPosition();
	float radius = p_Sphere.getRadius();

	if (spherePos.x + radius > p_Max.x) return false;
	if (spherePos.x - radius < p_Min.x)	return false;
	if (spherePos.y + radius > p_Max.y) return false;
	if (spherePos.y - radius < p_Min.y) return false;
	if (spherePos.z + radius > p_Max.z) return false;
	if (spherePos.z - radius < p_Min.z) return false;

	return true;
}

HitData Collision::OBBvsOBB(OBB const &p_OBB1, OBB const &p_OBB2)
{
	if(!surroundingSphereVsSphere(p_OBB1.getSphere(), p_OBB2.getSphere()))
		return HitData();

	return SATBoxVsBox(p_OBB1, p_OBB2);
}

HitData Collision::OBBvsSphere(OBB const &p_OBB, Sphere const &p_Sphere)
{
	if(!surroundingSphereVsSphere(p_OBB.getSphere(), p_Sphere))
		return HitData();

	HitData hit;

	XMVECTOR sphereCent = XMLoadFloat4(&p_Sphere.getPosition());	// m
	
	XMVECTOR closestPoint = p_OBB.findClosestPt(sphereCent);	// m

	XMVECTOR v = closestPoint - sphereCent;	// m
	XMVECTOR vv = XMVector4Dot(v, v);	// m^2

	if(XMVectorGetX(vv) <= p_Sphere.getSqrRadius())
	{
		hit.intersect = true;
		hit.colPos.x = XMVectorGetX(closestPoint) * 100.f;
		hit.colPos.y = XMVectorGetY(closestPoint) * 100.f;
		hit.colPos.z = XMVectorGetZ(closestPoint) * 100.f;
		hit.colPos.w = 1.f;

		XMVECTOR tempNorm = XMVector4Normalize(XMLoadFloat4(&p_Sphere.getPosition()) - closestPoint);
		float l = XMVectorGetX(XMVector4Length(tempNorm));
		if(l > XMVectorGetX(g_XMEpsilon))
			hit.colNorm = tempNorm;
		else
		{
			XMVECTOR n = sphereCent - XMLoadFloat4(&p_OBB.getPosition());
			n = XMVectorSetW(n, 0.f);
			hit.colNorm = XMVector4Normalize(n);
		}
			
		
		hit.colLength = (p_Sphere.getRadius() - sqrtf(vv.m128_f32[0]));
		hit.colType = Type::OBBVSSPHERE;
	}

	return hit;
}

HitData Collision::OBBvsAABB(OBB const &p_OBB, AABB const &p_AABB)
{
	if(!surroundingSphereVsSphere(p_OBB.getSphere(), p_AABB.getSphere()))
		return HitData();

	return SATBoxVsBox(p_OBB, p_AABB);
}

HitData Collision::OBBVsHull(OBB const &p_OBB, Hull const &p_Hull)
{
	if(!surroundingSphereVsSphere(p_OBB.getSphere(), p_Hull.getSphere()))
		return HitData();

	return SATBoxVsHull(p_OBB, p_Hull);
}

HitData Collision::HullVsSphere(Hull const &p_Hull, Sphere const &p_Sphere)
{
	if(!surroundingSphereVsSphere(p_Hull.getSphere(), p_Sphere))
		return HitData();

	HitData hit;
	XMFLOAT4 XMSpherePos = p_Sphere.getPosition();
	XMVECTOR spherePos = XMLoadFloat4(&XMSpherePos);

	float distance = FLT_MAX;
	XMVECTOR closestPoint = g_XMZero;
	unsigned int nrTriangles = p_Hull.getTriangleListSize();
	for(unsigned int i = 0; i < nrTriangles; i++)
	{
		XMVECTOR point = p_Hull.findClosestPointOnTriangle(XMSpherePos, i);
		XMVECTOR v = point - spherePos;

		float vv = XMVectorGetX(XMVector4Dot(v, v));

		if(vv <= p_Sphere.getSqrRadius())
		{
			hit.intersect = true;
			if(vv <= distance)
			{
				distance = vv;
				closestPoint = point;
			}
		}
	}

	if(hit.intersect)
	{
		hit.colPos.x = XMVectorGetX(closestPoint) * 100.f;
		hit.colPos.y = XMVectorGetY(closestPoint) * 100.f;
		hit.colPos.z = XMVectorGetZ(closestPoint) * 100.f;
		hit.colPos.w = 1.f;

		XMVECTOR tempNorm = XMVector4Normalize(spherePos - closestPoint);


		float l = XMVectorGetX(XMVector4Length(tempNorm));
		if(l > XMVectorGetX(g_XMEpsilon))
			hit.colNorm = tempNorm;
		else
		{
			XMVECTOR n = spherePos - XMLoadFloat4(&p_Hull.getPosition());
			n = XMVectorSetW(n, 0.f);
			hit.colNorm = XMVector4Normalize(n);
		}
		hit.colLength = (p_Sphere.getRadius() - sqrtf(distance));
		hit.colType = Type::HULLVSSPHERE;

	}
	return hit;
}

HitData Collision::SATBoxVsBox(OBB const &p_OBB, BoundingVolume const &p_vol)
{
	HitData miss;
	float r, ra, rb, overlap = FLT_MAX;

	XMMATRIX R, AbsR;
	XMVECTOR b_Center, b_Extents; // m
	XMMATRIX b_Axes;
	XMVECTOR least;
	const XMVECTOR a_Center = XMLoadFloat4(&p_OBB.getPosition());
	const XMMATRIX a_Axes = XMLoadFloat4x4(&p_OBB.getAxes());
	const XMVECTOR a_Extents = XMLoadFloat4(&p_OBB.getExtents()); 


	if(p_vol.getType() == BoundingVolume::Type::OBB)
	{
		b_Center = XMLoadFloat4(&((OBB&)p_vol).getPosition());
		b_Axes = XMLoadFloat4x4(&((OBB&)p_vol).getAxes());
		b_Extents = XMLoadFloat4(&((OBB&)p_vol).getExtents());
	}
	else
	{
		b_Center = XMLoadFloat4(&((AABB&)p_vol).getPosition());
		b_Axes = XMMatrixIdentity();
		b_Extents = XMLoadFloat4(&((AABB&)p_vol).getHalfDiagonal());
	}
	//Compute rotation matrix expressing b in a's coordinate frame
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			XMVECTOR dotResult = XMVector3Dot(a_Axes.r[i], b_Axes.r[j]);
			R.r[i].m128_f32[j] = XMVectorGetX(dotResult);
		}
	}

	// Compute translation vector t
	XMVECTOR t = b_Center - a_Center;	// m
	XMVECTOR tVec = t;
	
	// Bring translation into a�s coordinate frame
	XMVECTOR dotResult = XMVector3Dot(t, a_Axes.r[0]); 
	XMVECTOR dotResult1 = XMVector3Dot(t, a_Axes.r[1]); 
	XMVECTOR dotResult2 = XMVector3Dot(t, a_Axes.r[2]); 
	t = XMVectorSet(XMVectorGetX(dotResult), XMVectorGetX(dotResult1), XMVectorGetX(dotResult2), 0.f);

	// Compute common subexpressions. Add in an epsilon term to
	// counteract arithmetic errors when two edges are parallel and
	// their cross product is (near) null 
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			AbsR.r[i].m128_f32[j] = fabs(R.r[i].m128_f32[j]) + EPSILON;
		}
	}

	// Test axes L = A0, L = A1, L = A2
	for (int i = 0; i < 3; i++) 
	{
		ra = a_Extents.m128_f32[i]; 
		rb = XMVectorGetX(b_Extents) * XMVectorGetX(AbsR.r[i]) + XMVectorGetY(b_Extents) * XMVectorGetY(AbsR.r[i]) + XMVectorGetZ(b_Extents) * XMVectorGetZ(AbsR.r[i]);
		float r = t.m128_f32[i];
		if(fabs(r) > ra + rb)
			return miss;
		checkCollisionDepth(ra, rb, r, overlap, a_Axes.r[i], least);
		
	}

	//Test axes L = B0, L = B1, L = B2
	for (int i = 0; i < 3; i++) 
	{
		ra = XMVectorGetX(a_Extents) * AbsR.r[0].m128_f32[i] + XMVectorGetY(a_Extents) * AbsR.r[1].m128_f32[i] + XMVectorGetZ(a_Extents) * AbsR.r[2].m128_f32[i];
		rb = b_Extents.m128_f32[i]; 
		r  = XMVectorGetX(t) * R.r[0].m128_f32[i] + XMVectorGetY(t) * R.r[1].m128_f32[i] + XMVectorGetZ(t) * R.r[2].m128_f32[i];
		if(fabs(r) > ra + rb)
			return miss;
		checkCollisionDepth(ra, rb, r, overlap, b_Axes.r[i], least);
	}

	// Test axis L = A0 x B0
	ra		= XMVectorGetY(a_Extents) * XMVectorGetX(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetX(AbsR.r[1]);
	rb		= XMVectorGetY(b_Extents) * XMVectorGetZ(AbsR.r[0]) + XMVectorGetZ(b_Extents) * XMVectorGetY(AbsR.r[0]);
	r		= XMVectorGetZ(t) * XMVectorGetX(R.r[1]) - XMVectorGetY(t) * XMVectorGetX(R.r[2]);
	if (fabs(r) > ra + rb)
		return miss;

	XMVECTOR L = XMVector3Cross(a_Axes.r[0], b_Axes.r[0]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A0 x B1
 	ra		= XMVectorGetY(a_Extents)	* XMVectorGetY(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetY(AbsR.r[1]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetZ(AbsR.r[0]) + XMVectorGetZ(b_Extents) * XMVectorGetX(AbsR.r[0]);
	r		= XMVectorGetZ(t) * XMVectorGetY(R.r[1]) - XMVectorGetY(t) * XMVectorGetY(R.r[2]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[0], b_Axes.r[1]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A0 x B2
	ra		= XMVectorGetY(a_Extents)	* XMVectorGetZ(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetZ(AbsR.r[1]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetY(AbsR.r[0]) + XMVectorGetY(b_Extents) * XMVectorGetX(AbsR.r[0]);
	r		= XMVectorGetZ(t) * XMVectorGetZ(R.r[1]) - XMVectorGetY(t) * XMVectorGetZ(R.r[2]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[0], b_Axes.r[2]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A1 x B0
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetX(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetX(AbsR.r[0]);
	rb		= XMVectorGetY(b_Extents)	* XMVectorGetZ(AbsR.r[1]) + XMVectorGetZ(b_Extents) * XMVectorGetY(AbsR.r[1]);
	r		= XMVectorGetX(t) * XMVectorGetX(R.r[2]) - XMVectorGetZ(t) * XMVectorGetX(R.r[0]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[1], b_Axes.r[0]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A1 x B1
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetY(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetY(AbsR.r[0]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetZ(AbsR.r[1]) + XMVectorGetZ(b_Extents) * XMVectorGetX(AbsR.r[1]);
	r		= XMVectorGetX(t) * XMVectorGetY(R.r[2]) - XMVectorGetZ(t) * XMVectorGetY(R.r[0]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[1], b_Axes.r[1]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A1 x B2
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetZ(AbsR.r[2]) + XMVectorGetZ(a_Extents) * XMVectorGetZ(AbsR.r[0]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetY(AbsR.r[1]) + XMVectorGetY(b_Extents) * XMVectorGetX(AbsR.r[1]);
	r		= XMVectorGetX(t) * XMVectorGetZ(R.r[2]) - XMVectorGetZ(t) * XMVectorGetZ(R.r[0]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[1], b_Axes.r[2]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A2 x B0 
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetX(AbsR.r[1]) + XMVectorGetY(a_Extents) * XMVectorGetX(AbsR.r[0]);
	rb		= XMVectorGetY(b_Extents)	* XMVectorGetZ(AbsR.r[2]) + XMVectorGetZ(b_Extents) * XMVectorGetY(AbsR.r[2]);
	r		= XMVectorGetY(t) * XMVectorGetX(R.r[0]) - XMVectorGetX(t) * XMVectorGetX(R.r[1]);
	if(fabs(r) > ra + rb)
		return miss;

	L = XMVector3Cross(a_Axes.r[2], b_Axes.r[0]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A2 x B1
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetY(AbsR.r[1]) + XMVectorGetY(a_Extents) * XMVectorGetY(AbsR.r[0]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetZ(AbsR.r[2]) + XMVectorGetZ(b_Extents) * XMVectorGetX(AbsR.r[2]);
	r		= XMVectorGetY(t) * XMVectorGetY(R.r[0]) - XMVectorGetX(t) * XMVectorGetY(R.r[1]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[2], b_Axes.r[1]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);

	// Test axis L = A2 x B2
	ra		= XMVectorGetX(a_Extents)	* XMVectorGetZ(AbsR.r[1]) + XMVectorGetY(a_Extents) * XMVectorGetZ(AbsR.r[0]);
	rb		= XMVectorGetX(b_Extents)	* XMVectorGetY(AbsR.r[2]) + XMVectorGetY(b_Extents) * XMVectorGetX(AbsR.r[2]);
	r		= XMVectorGetY(t) * XMVectorGetZ(R.r[0]) - XMVectorGetX(t) * XMVectorGetZ(R.r[1]);
	if (fabs(r) > ra + rb) 
		return miss;

	L = XMVector3Cross(a_Axes.r[2], b_Axes.r[2]);
	checkCollisionDepth(ra, rb, r, overlap, L, least);
	
	float temp = XMVectorGetX(XMVector4Dot(tVec, least));

	if(temp > 0)
		least *= -1.f;
	HitData hit;
	hit.intersect = true;
	hit.colNorm = XMVector4Normalize(least);
	hit.colLength = overlap;

	if(p_vol.getType() == BoundingVolume::Type::OBB)
		hit.colType = Type::OBBVSOBB;
	else
		hit.colType = Type::OBBVSAABB;

 	return hit;
}

HitData Collision::SATBoxVsHull(OBB const &p_OBB, Hull const &p_Hull)
{
	HitData hit;

	//Box center
	const XMVECTOR C = XMLoadFloat4(&p_OBB.getPosition());
	//Box local axes
	const XMMATRIX A = XMLoadFloat4x4(&p_OBB.getAxes());
	//Box halfsize
	const XMVECTOR a = XMLoadFloat4(&p_OBB.getExtents());
	//Minimum translation vector
	XMVECTOR MTV = g_XMZero;
	//Stores the minimum translation vector for all triangles hit in a hull
	std::vector<XMFLOAT4> MTVs;

	for(unsigned int i = 0; i < p_Hull.getTriangleListSize(); i++)
	{
		//Triangle Vertices U0, U1 and U2.
		Triangle triangle = p_Hull.getTriangleInWorldCoord(i);
		XMVECTOR U0 = Vector4ToXMVECTOR(&triangle.corners[0]);
		XMVECTOR U1 = Vector4ToXMVECTOR(&triangle.corners[1]);
		XMVECTOR U2 = Vector4ToXMVECTOR(&triangle.corners[2]);

		//Triangle egdes E0, E1, E2
		XMVECTOR E0 = U1 - U0;
		XMVECTOR E1 = U2 - U0;
		XMVECTOR E2 = E1 - E0;

		//Triangle Normal
		XMVECTOR N = XMVector3Cross(E0, E1);

		//Vector from a box center to the fisrt vertex in the triangle
		XMVECTOR D = U0 - C;

		float overlap = FLT_MAX;

		//Least separating axis
		XMVECTOR least;
		
		//Axis L, Axis to test
		XMVECTOR L = N;

		//Triangle vertex projection on axis
		float p0 = XMVector3Dot(N, D).m128_f32[0];
		float p1;

		//Box projection on axis, the box interval on the axis is [-R, R]
		float R = XMVectorGetX(a) * fabs(XMVectorGetX(XMVector3Dot(N, A.r[0]))) 
				+ XMVectorGetY(a) * fabs(XMVectorGetX(XMVector3Dot(N, A.r[1])))
				+ XMVectorGetZ(a) * fabs(XMVectorGetX(XMVector3Dot(N, A.r[2])));



		//If the triangles projection interval [min, max] is outside the box interval [-R, R]
		//a separating axis is found which means no collision
		if(!checkCollision(L, p0, p0, R, overlap, least))
			continue;


		//Axis A0, A1, A2, The box local axes
		bool miss = false;
		for(int j = 0; j < 3; j++)
		{
			L = A.r[j];
			p0 = XMVectorGetX(XMVector3Dot(L, D));
			p1 = p0 + XMVectorGetX(XMVector3Dot(L, E0));
			float p2 = p0 + XMVectorGetX(XMVector3Dot(L, E1));
			R = a.m128_f32[j];
			float max = XMMax(p0, XMMax(p1,p2));
			float min = XMMin(p0, XMMin(p1,p2));
			

			if(!checkCollision(L, min, max, R, overlap, least))
			{
				miss = true;
				break;
			}
		}
		if(miss)
			continue;

		//L = A0 x E0
		L = XMVector3Cross(A.r[0], E0);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 + XMVector3Dot(A.r[0], N).m128_f32[0];
		R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E0).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E0).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// L = A0 x E1
		L = XMVector3Cross(A.r[0], E1);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[0], N).m128_f32[0];
		R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E1).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E1).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// L = A0 x E2
		L = XMVector3Cross(A.r[0], E2);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[0], N).m128_f32[0];
		R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E2).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E2).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// L = A1 x E0
		L = XMVector3Cross(A.r[1], E0);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 + XMVector3Dot(A.r[1], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E0).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E0).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// L = A1 x E1
		L = XMVector3Cross(A.r[1], E1);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[1], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E1).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E1).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// L = A1 x E2
		L = XMVector3Cross(A.r[1], E2);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[1], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E2).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E2).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// A2 x E0
		L = XMVector3Cross(A.r[2], E0);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 + XMVector3Dot(A.r[2], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E0).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E0).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// A2 x E1
		L = XMVector3Cross(A.r[2], E1);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[2], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E1).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E1).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		// A2 x E2
		L = XMVector3Cross(A.r[2], E2);
		p0 = XMVector3Dot(L, D).m128_f32[0];
		p1 = p0 - XMVector3Dot(A.r[2], N).m128_f32[0];
		R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E2).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E2).m128_f32[0]);
		if(!checkCollision(L, p0, p1, R, overlap, least))
			continue;

		//Check if the triangle and box are side by side, if they are, it counts as no collision
		if (overlap == 0.f)
			continue;


		//No separating axis found, The box is colliding with a triangle.
		hit.intersect = true;
		hit.colType = Type::OBBVSHULL;

		//Minimum translation vector for this triangle in the hull.
		XMVECTOR triangleMTV = least * overlap;

		XMVECTOR trianglePoint = p_Hull.findClosestPointOnTriangle(p_OBB.getPosition(), i);
		XMVECTOR boxCenterToTriangle = trianglePoint - C;

		//Check the direction of the MTV to see if we need to change it.
		float dirCheck = XMVectorGetX(XMVector4Dot(boxCenterToTriangle, triangleMTV));
		if(dirCheck > 0)
			triangleMTV *= -1.f;

		bool exist = false;
		for(unsigned int j = 0; j < MTVs.size(); j++)
		{
			XMVECTOR norm1 = XMVector4Normalize(triangleMTV);
			XMVECTOR norm2 = XMVector4Normalize(XMLoadFloat4(&MTVs[j]));

			float dotNorm = XMVectorGetX(XMVector4Dot(norm1, norm2));
			//Check if this triangle MTV has the same direction as a previusly found MTV
			if(dotNorm >= 1.f - EPSILON && dotNorm <= 1.f + EPSILON)
			{
				exist = true;
				break;
			}
		}

		if(exist)
			continue;
		
		//Save this triangle MTV so it can be used to filter out MTVs that point in the same direction.
		XMFLOAT4 tempVec;
		XMStoreFloat4(&tempVec, triangleMTV);
		MTVs.push_back(tempVec);

		//Add to our MTV
		MTV += triangleMTV;
	}

	if(hit.intersect)
	{
		//the length of the MTV is our collision depth
		float depth = XMVectorGetX(XMVector4Length(MTV));

		//The normalized MTV is our collision normal 
		MTV = XMVector4Normalize(MTV);

		hit.colNorm = MTV;
		hit.colLength = depth;
	}
	return hit;

}

void Collision::checkCollisionDepth(float p_RA, float p_RB, float p_R, float &p_Overlap, XMVECTOR p_L, XMVECTOR &p_Least)
{
	float lLength = XMVectorGetX(XMVector4LengthSq(p_L));
	if(lLength > EPSILON)
	{
		p_R = (fabs(p_R) - (p_RA + p_RB)) / lLength;
		if(p_Overlap > fabs(p_R))
		{
			p_Overlap = fabs(p_R);
			p_Least = p_L;
		}
	}
}

bool Collision::checkCollision(XMVECTOR p_Axis, float p_TriangleProjection0, float p_TriangleProjection1, float p_BoxProjection, float &p_Overlap, XMVECTOR &p_Least)
{
	float triangleMin = XMMin(p_TriangleProjection0, p_TriangleProjection1);
	float triangleMax = XMMax(p_TriangleProjection0, p_TriangleProjection1);

	if(triangleMax < -p_BoxProjection || triangleMin > p_BoxProjection )
		return false;
	

	float lLength = XMVectorGetX(XMVector4LengthSq(p_Axis));
	if(lLength > EPSILON)
	{
		float overlap;
		float d0 = fabs(p_BoxProjection - triangleMin) / lLength;
		float d1 = fabs(triangleMax - (-p_BoxProjection)) / lLength;

		if(d0 < d1)
			overlap = d0;
		else
			overlap = d1;

		if(p_Overlap > fabs(overlap))
		{
			p_Overlap = fabs(overlap);
			p_Least = p_Axis;
		}
	}
	return true;
}

float Collision::raySphereIntersect(const Sphere &p_Sphere, const XMFLOAT4 &p_RayDirection, const XMFLOAT4 &p_RayOrigin)
{

	//Intersection test starts

	//Transform to object space?
	const XMVECTOR spherePos = XMLoadFloat4(&p_Sphere.getPosition());
	const XMVECTOR rayDir = XMLoadFloat4(&p_RayDirection);
	const XMVECTOR rayOrigin = XMLoadFloat4(&p_RayOrigin);

	const XMVECTOR length = spherePos - rayOrigin;
	////projection of lenght onto ray direction
	float s = XMVector3Dot(length, rayDir).m128_f32[0];

	float lengthSquared = XMVector3Dot(length, length).m128_f32[0];
	float radiusSquared = p_Sphere.getSqrRadius();

	if(s < 0 && lengthSquared > radiusSquared)
		return -1.f; //miss

	////squared distance from sphere center to projection
	float m = lengthSquared - (s*s);

	if(m > radiusSquared)
		return -1.f; //miss
	//
	float q = radiusSquared - m;
	q = sqrtf(q);

	float t;
	if(lengthSquared > radiusSquared)
	{
		t = s - q;
	}
	else
	{
		t = s + q;
	}

	return t;
}

float Collision::rayTriangleIntersect(const Hull &p_Hull, const XMFLOAT4 &p_RayDirection, const XMFLOAT4 &p_RayOrigin)
{
	XMVECTOR RayDir = XMLoadFloat4(&p_RayDirection);
	XMVECTOR RayOrigin = XMLoadFloat4(&p_RayOrigin);
	float dist = FLT_MAX;

	for(unsigned int i = 0; i < p_Hull.getTriangleListSize(); i++)
	{
		Triangle tri = p_Hull.getTriangleAt(i);
		float tempDist = 0.f;
		//Triangle Vertices 
		Triangle triangle = p_Hull.getTriangleInWorldCoord(i);
		const XMVECTOR p0 = Vector4ToXMVECTOR(&triangle.corners[0]);
		const XMVECTOR p1 = Vector4ToXMVECTOR(&triangle.corners[1]);
		const XMVECTOR p2 = Vector4ToXMVECTOR(&triangle.corners[2]);

		//Triangle egdes 
		const XMVECTOR e1 = p1 - p0;
		const XMVECTOR e2 = p2 - p0;

		XMVECTOR q = XMVector3Cross(RayDir, e2);
		float a = XMVector3Dot(e1, q).m128_f32[0];
		if(a > -EPSILON && a < EPSILON)
			continue;

		float f = 1/a; //because math!

		XMVECTOR s = RayOrigin - p0;
		float u = f * XMVector3Dot(s, q).m128_f32[0];
		if(u < 0.f)
			continue;

		XMVECTOR r = XMVector3Cross(s, e1);
		float v = f * XMVector3Dot(RayDir, r).m128_f32[0];
		if(v < 0.f || (u + v) > 1.f)
			continue;

		float t = f * XMVector3Dot(e2, r).m128_f32[0];

		if(t > 0.f && t < dist)
		{
			dist = t;
		}
	}

	if(dist == FLT_MAX)
		return -1.f;
	else
		return dist;
}