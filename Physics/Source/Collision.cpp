#include "Collision.h"
#define EPSILON XMVectorGetX(g_XMEpsilon)
using namespace DirectX;



HitData Collision::boundingVolumeVsBoundingVolume(BoundingVolume const &p_Volume1, BoundingVolume const &p_Volume2)
{
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
		HitData hit = HitData();
		return hit;
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
		HitData hit = HitData();
		return hit;
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
	case BoundingVolume::Type::HULL:
		return OBBVsHull(p_AABB, (Hull&)p_Volume);
	default:
		HitData hit = HitData();
		return hit;
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
	//case BoundingVolume::Type::HULL:
	//	return OBBVsHull(p_OBB, (Hull&)p_Volume);
	default:
		HitData hit = HitData();
		return hit;
	}
}

HitData Collision::boundingVolumeVsHull(BoundingVolume const &p_Volume, Hull const &p_Hull)
{
	BoundingVolume::Type type = p_Volume.getType();
	switch(type)
	{
	case BoundingVolume::Type::SPHERE:
			return HullVsSphere(p_Hull, (Sphere&)p_Volume);
	case BoundingVolume::Type::AABBOX:
		return OBBVsHull((AABB&)p_Volume, p_Hull);
	default:
		HitData hit = HitData();
		return hit;
	}
}

HitData Collision::sphereVsSphere(Sphere const &p_Sphere1, Sphere const &p_Sphere2 )
{
	HitData hit = HitData();

    XMVECTOR CDiff, vPos;	// m
	CDiff = XMLoadFloat4(&p_Sphere2.getPosition());
	vPos = XMLoadFloat4(&p_Sphere1.getPosition());
	CDiff = CDiff - vPos;
	
	float c = XMVector3LengthSq(CDiff).m128_f32[0];	// m^2
	float rSum = p_Sphere2.getRadius() + p_Sphere1.getRadius();	// m
    float rSumSqr = rSum*rSum;	// m^2

	//Find out if the sphere centers are separated with more distance than the radiuses.
    if(c <= rSumSqr)
	{
		hit.intersect = true;

		XMFLOAT4 position;	// m
		
		XMVECTOR normalized = XMVector4Normalize( XMLoadFloat4(&p_Sphere1.getPosition()) - XMLoadFloat4(&p_Sphere2.getPosition()));
		XMVECTOR hitPos = normalized  * p_Sphere2.getRadius();	// m

		hit.colPos = XMVECTORToVector4(&hitPos) * 100.f;

		hit.colNorm.x = normalized.m128_f32[0];
		hit.colNorm.y = normalized.m128_f32[1];
		hit.colNorm.z = normalized.m128_f32[2];
		hit.colNorm.w = normalized.m128_f32[3];

		hit.colLength = (rSum - sqrtf(c)) * 100.f;

		hit.colType = Type::SPHEREVSSPHERE;
	}

	return hit;
}

HitData Collision::AABBvsAABB(AABB const &p_AABB1, AABB const &p_AABB2 )
{	
	HitData hit = sphereVsSphere(p_AABB1.getSphere(), p_AABB2.getSphere());
	if(!hit.intersect)
		return hit;

	hit = HitData();
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
	HitData hit = sphereVsSphere(p_AABB.getSphere(), p_Sphere); 
	if(!hit.intersect)
		return hit;

	hit = HitData();

	//Check to see if the sphere overlaps the AABB
	//const bool AABBOverlapsSphere ( const AABB& B, const SCALAR r, VECTOR& C )
	float s = 0; // m
	float d = 0; // m^2

	//find the square of the distance
	//from the sphere to the box
	XMFLOAT4 spherePos = p_Sphere.getPosition();	// m
	XMFLOAT3 dist = XMFLOAT3(.0f, .0f, .0f);	// m

	//if the sphere is outside of the box, find the corner closest to the sphere center in each axis.
	//else special case for when the sphere center is inside that axis slab.

	XMFLOAT4 bMin;	// m
	XMStoreFloat4( &bMin, XMLoadFloat4(&p_AABB.getMin()));
	XMFLOAT4 bMax;	// m
	XMStoreFloat4( &bMax, XMLoadFloat4(&p_AABB.getMax()));
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

		XMFLOAT4 colPos(dist.x, dist.y, dist.z, 1.f);
		XMVECTOR tempNorm = XMVector4Normalize( XMLoadFloat4(&p_Sphere.getPosition()) - XMLoadFloat4(&colPos));



		hit.colNorm = XMVECTORToVector4(&tempNorm);
		hit.colLength = (p_Sphere.getRadius() - sqrtf(d)) * 100.f;

		hit.colType = Type::AABBVSSPHERE;
	}

	return hit;
}

HitData Collision::OBBvsOBB(OBB const &p_OBB1, OBB const &p_OBB2)
{
	HitData hit = sphereVsSphere(p_OBB1.getSphere(), p_OBB2.getSphere());
	if(!hit.intersect)
		return hit;

	return seperatingAxisTest(p_OBB1, p_OBB2);
}

HitData Collision::OBBvsSphere(OBB const &p_OBB, Sphere const &p_Sphere)
{
	HitData hit = sphereVsSphere(p_OBB.getSphere(), p_Sphere);
	if(!hit.intersect)
		return hit;

	hit = HitData();

	XMFLOAT4 dist;	// m
	

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
			hit.colNorm = XMVECTORToVector4(&tempNorm);
		else
		{
			XMVECTOR n = sphereCent - XMLoadFloat4(&p_OBB.getPosition());
			n = XMVectorSetW(n, 0.f);
			hit.colNorm = XMVECTORToVector4(&XMVector4Normalize(n));
		}
			
		
		hit.colLength = (p_Sphere.getRadius() - sqrtf(vv.m128_f32[0])) * 100.f;
		hit.colType = Type::OBBVSSPHERE;
	}

	return hit;
}

HitData Collision::OBBvsAABB(OBB const &p_OBB, AABB const &p_AABB)
{
	HitData hit = sphereVsSphere(p_OBB.getSphere(), p_AABB.getSphere());
	if(!hit.intersect)
		return hit;

	return seperatingAxisTest(p_OBB, p_AABB);
}

HitData Collision::HullVsSphere(Hull const &p_Hull, Sphere const &p_Sphere)
{
	HitData hit = sphereVsSphere(p_Hull.getSphere(), p_Sphere);
	if(!hit.intersect)
		return hit;

	hit = HitData();

	XMVECTOR closestPoint, v, spherePos, point;

	spherePos = XMLoadFloat4(&p_Sphere.getPosition());

	float distance = FLT_MAX;

	for(unsigned int i = 0; i < p_Hull.getTriangleListSize(); i++)
	{
		Triangle tri = p_Hull.getTriangleInWorldCoord(i);

		point = XMLoadFloat4(&findClosestPointOnTriangle(p_Sphere.getPosition(), tri.corners[0], tri.corners[1], tri.corners[2]));
		//point = XMLoadFloat4(&p_Hull.findClosestPointOnTriangle(p_Sphere.getPosition(), i));
		v = point - spherePos;

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

		XMVECTOR tempNorm = XMVector4Normalize(XMLoadFloat4(&p_Sphere.getPosition()) - closestPoint);
		float l = XMVectorGetX(XMVector4Length(tempNorm));
		if(l > XMVectorGetX(g_XMEpsilon))
			hit.colNorm = XMVECTORToVector4(&tempNorm);
		else
		{
			XMVECTOR n = spherePos - XMLoadFloat4(&p_Hull.getPosition());
			n = XMVectorSetW(n, 0.f);
			hit.colNorm = XMVECTORToVector4(&XMVector4Normalize(n));
		}
		hit.colLength = (p_Sphere.getRadius() - sqrtf(distance)) * 100.f;
		hit.colType = Type::HULLVSSPHERE;

	}


	return hit;
}

bool Collision::SphereVsTriangle(Sphere const &p_Sphere, Triangle const &p_Triangle)
{
	return false;
}

HitData Collision::seperatingAxisTest(OBB const &p_OBB, BoundingVolume const &p_vol)
{
	HitData miss = HitData();
	float r, ra, rb, overlap = FLT_MAX;
	//const float EPSILON = XMVectorGetX(g_XMEpsilon);
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
	HitData hit = HitData();
	hit.intersect = true;
	hit.colNorm = XMVECTORToVector4(&XMVector4Normalize(least));
	hit.colLength = overlap * 100.f;

	if(p_vol.getType() == BoundingVolume::Type::OBB)
		hit.colType = Type::OBBVSOBB;
	else
		hit.colType = Type::OBBVSAABB;

 	return hit;
}

void Collision::checkCollisionDepth(float p_RA, float p_RB, float p_R, float &p_Overlap, XMVECTOR p_L, XMVECTOR &p_Least)
{
	float lLength = XMVectorGetX(XMVector4LengthSq(p_L));
	if(lLength > XMVectorGetX(g_XMEpsilon))
	{
		p_R = (fabs(p_R) - (p_RA + p_RB)) / lLength;
		if(p_Overlap > fabs(p_R))
		{
			p_Overlap = fabs(p_R);
			p_Least = p_L;
		}
	}
}

void Collision::checkCollisionDepth(float p_RA, float p_RB, float p_RC ,float p_R, float &p_Overlap, XMVECTOR p_L, XMVECTOR &p_Least)
{
	float lLength = XMVectorGetX(XMVector4LengthSq(p_L));
	if(lLength > XMVectorGetX(g_XMEpsilon))
	{
		p_R = (fabs(p_R) - (p_RA + p_RB+ p_RC)) / lLength;
		if(p_Overlap > fabs(p_R))
		{
			p_Overlap = fabs(p_R);
			p_Least = p_L;
		}
	}
}

HitData Collision::OBBVsHull(AABB const &p_OBB, Hull const &p_Hull)
{
	HitData hit = sphereVsSphere(p_OBB.getSphere(), p_Hull.getSphere());
	if(!hit.intersect)
	{
		return hit;
	}

	hit = HitData();

	const XMVECTOR box_Center   = XMLoadFloat4(&p_OBB.getPosition());
	const XMMATRIX box_Axes		= XMMatrixIdentity();//(&p_OBB.getAxes());
	const XMVECTOR box_Extents  = XMLoadFloat4(&p_OBB.getHalfDiagonal());
	const XMMATRIX invAxes		= XMMatrixInverse(&XMMatrixDeterminant(box_Axes), box_Axes); 	//not used

	bool miss;
	float overlap = FLT_MAX;
	XMVECTOR least = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	for(unsigned int i = 0; i < p_Hull.getTriangleListSize(); i++)
	{
		
#pragma region eberly

		/*eberly
		Triangle Vertices U0, U1 and U2.
		E0 = U1 - U0,
		E1 = U2 - U0,
		E2 = E1 - E0,
		N  = E0 X E1,
		D  = U0 - C,
		Obb center = C
		Obb axes = A0, A1, A2
		obb extents = a1, a2, a3*/
		//Triangle tri = p_Hull.getTriangleInWorldCoord(i);
		//XMVECTOR U0 = Vector4ToXMVECTOR(&tri.corners[0]);
		//XMVECTOR U1 = Vector4ToXMVECTOR(&tri.corners[1]);
		//XMVECTOR U2 = Vector4ToXMVECTOR(&tri.corners[2]);

		//XMVECTOR E0 = U1 - U0;
		//XMVECTOR E1 = U2 - U0;
		//XMVECTOR E2 = E1 - E0;
		//XMVECTOR D = U0 - C;
		//XMVECTOR N = XMVector3Cross(E0, E1);
		//N = XMVector4Normalize(N);
		//
		////Axis L = N
		//XMVECTOR L  = N;
		//float p0 = XMVector3Dot(N, D).m128_f32[0];
		//float p1 = p0;
		//float p2 = p0;
		//XMVECTOR NA0 = XMVector3Dot(N, A.r[0]);
		//XMVECTOR NA1 = XMVector3Dot(N, A.r[1]);
		//XMVECTOR NA2 = XMVector3Dot(N, A.r[2]);

		//float R  = XMVectorGetX(a) * fabs(XMVectorGetX(NA0)) + XMVectorGetY(a) * fabs(XMVectorGetX(NA1)) + XMVectorGetZ(a) * fabs(XMVectorGetX(NA2));
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		////Axis A0, A1, A2
		//bool miss = false;
		//for(int j = 0; j < 3; j++)
		//{
		//	p0 = XMVector3Dot(A.r[j], D).m128_f32[0];
		//	p1 = p0 + XMVector3Dot(A.r[j], E0).m128_f32[0];
		//	p2 = p0 + XMVector3Dot(A.r[j], E1).m128_f32[0];
		//	float R = a.m128_f32[j];

		//	if(checkR(p0, p1, p2, R))
		//	{
		//		miss = true;
		//		break;
		//	}
		//	else
		//		checkCollisionDepth(p0, p1, p2, R, overlap, A.r[j], least);
		//}
		//if(miss)
		//	continue;

		////L = A0 x E0
		//L = XMVector3Cross(A.r[0], E0);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0;
		//p2 = p0 + XMVector3Dot(A.r[0], N).m128_f32[0];
		//R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E0).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E0).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// L = A0 x E1
		//L = XMVector3Cross(A.r[0], E1);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[0], N).m128_f32[0];
		//p2 = p0;
		//R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E1).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E1).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// L = A0 x E2
		//L = XMVector3Cross(A.r[0], E2);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[0], N).m128_f32[0];
		//p2 = p0 - XMVector3Dot(A.r[0], N).m128_f32[0];
		//R = XMVectorGetY(a) * fabs(XMVector3Dot(A.r[2], E2).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[1], E2).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// L = A1 x E0
		//L = XMVector3Cross(A.r[1], E0);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0;
		//p2 = p0 + XMVector3Dot(A.r[1], N).m128_f32[0];
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E0).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E0).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// L = A1 x E1
		//L = XMVector3Cross(A.r[1], E1);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[1], N).m128_f32[0];
		//p2 = p0;
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E1).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E1).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// L = A1 x E2
		//L = XMVector3Cross(A.r[1], E2);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[1], N).m128_f32[0];
		//p2 = p0 - XMVector3Dot(A.r[1], N).m128_f32[0];
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[2], E2).m128_f32[0]) + XMVectorGetZ(a) * fabs(XMVector3Dot(A.r[0], E2).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// A2 x E0
		//L = XMVector3Cross(A.r[2], E0);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0;
		//p2 = p0 + XMVector3Dot(A.r[2], N).m128_f32[0];
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E0).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E0).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// A2 x E1
		//L = XMVector3Cross(A.r[2], E1);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[2], N).m128_f32[0];
		//p2 = p0;
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E1).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E1).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

		//// A2 x E2
		//L = XMVector3Cross(A.r[2], E2);
		//p0 = XMVector3Dot(L, D).m128_f32[0];
		//p1 = p0 - XMVector3Dot(A.r[2], N).m128_f32[0];
		//p2 = p0 - XMVector3Dot(A.r[2], N).m128_f32[0];
		//R = XMVectorGetX(a) * fabs(XMVector3Dot(A.r[1], E2).m128_f32[0]) + XMVectorGetY(a) * fabs(XMVector3Dot(A.r[0], E2).m128_f32[0]);
		//if(checkR(p0, p1, p2, R))
		//	continue;
		//checkCollisionDepth(p0, p1,p2,R, overlap, L, least);

#pragma endregion

		float tOverlap = FLT_MAX;
		XMVECTOR tLeast = g_XMZero;

		Triangle tri = p_Hull.getTriangleInWorldCoord(i);
		Triangle tri1 = p_Hull.getTriangleAt(i);
		XMVECTOR v0 = Vector4ToXMVECTOR(&tri.corners[0]);
		XMVECTOR v1 = Vector4ToXMVECTOR(&tri.corners[1]);
		XMVECTOR v2 = Vector4ToXMVECTOR(&tri.corners[2]);

		//v0 = XMVector4Transform(v0, invAxes);
		//v1 = XMVector4Transform(v1, invAxes);
		//v2 = XMVector4Transform(v2, invAxes);

		v0 = v0 - box_Center;
		v1 = v1 - box_Center;
		v2 = v2 - box_Center;

		XMVECTOR F[] = { v1 - v0,
						 v2 - v1,
						 v0 - v2, };

		float p0, p1, p2, r;
		miss = false;

		//Check axes Aj X Fk 
		for(int j = 0; j < 3; j++)
		{
			for(int k = 0; k < 3; k++)
			{
				XMVECTOR a = XMVector3Cross(box_Axes.r[j], F[k]);
				//r = e0 * fabs(A0 * Aj X Fk) + e1 * fabs(A1 * Aj X Fk) + e2 * fabs(A2 * Aj X Fk) + EPSILON
				r =	  box_Extents.m128_f32[0] * fabs(XMVector3Dot(box_Axes.r[0], a).m128_f32[0])
					+ box_Extents.m128_f32[1] * fabs(XMVector3Dot(box_Axes.r[1], a).m128_f32[0])
					+ box_Extents.m128_f32[2] * fabs(XMVector3Dot(box_Axes.r[2], a).m128_f32[0]) + EPSILON;

				p0 = XMVector3Dot(v0, a).m128_f32[0] + EPSILON;
				p1 = XMVector3Dot(v1, a).m128_f32[0] + EPSILON;
				p2 = XMVector3Dot(v2, a).m128_f32[0] + EPSILON;

				if(XMMax(-max(p0,p1,p2), min(p0,p1,p2)) > r)
				{
					j = 3;
					miss = true;
					break;
				}
				checkCollisionDepth(p0, p1, p2, r, tOverlap, a, tLeast);
			}
		}

		if(miss)
			continue;

		//Check axis A0
		if(checkR(v0.m128_f32[0], v1.m128_f32[0], v2.m128_f32[0], box_Extents.m128_f32[0]))
			continue;
		checkCollisionDepth(v0.m128_f32[0], v1.m128_f32[0], v2.m128_f32[0], box_Extents.m128_f32[0], tOverlap, box_Axes.r[0], tLeast);
		//Check axis A1
		if(checkR(v0.m128_f32[1], v1.m128_f32[1], v2.m128_f32[1], box_Extents.m128_f32[1]))
			continue;
		checkCollisionDepth(v0.m128_f32[1], v1.m128_f32[1], v2.m128_f32[1], box_Extents.m128_f32[1], tOverlap, box_Axes.r[1], tLeast);
		//Check axis a2
		if(checkR(v0.m128_f32[2], v1.m128_f32[2], v2.m128_f32[2], box_Extents.m128_f32[2]))
			continue;
		checkCollisionDepth(v0.m128_f32[2], v1.m128_f32[2], v2.m128_f32[2], box_Extents.m128_f32[2], tOverlap, box_Axes.r[2], tLeast);

		//Check for degenerate triangle
		float degenerate = XMVector4Length(XMVector3Cross(F[0], F[1])).m128_f32[0] * 0.5f;
		
		if(degenerate > EPSILON)
		{
			//Check axis triangle face normal.
			Plane p;
			XMStoreFloat4(&p.normal, XMVector3Cross(F[0], F[1]));
			p.d = XMVector3Dot(XMLoadFloat4(&p.normal), v0).m128_f32[0];
			if(!AABBVsPlane(p_OBB, p))
				continue;
		}
		
		hit.intersect = true;
		//Check if this triangle is closer than the last, if so save the least separeting axis.
		if(tOverlap <= overlap)
		{
			overlap = tOverlap;
			least = tLeast;
		}
	}

	least = XMVector4Normalize(least);
	hit.colNorm = XMVECTORToVector4(&least);// Vector4(0.f, 1.f, 0.f, 0.f);
	hit.colLength = overlap * 100.f;

	return hit;
}

bool Collision::checkR(float p0, float p1, float p2, float R)
{
	if(min(p0, p1, p2) > R || max(p0, p1, p2) < -R)
		return true;
	return false;
}

float Collision::min(const float &p_A, const float &p_B, const float &p_C)
{
	return (p_A < p_B) ? (p_A < p_C) ? p_A : p_C : (p_B < p_C) ? p_B : p_C;
}

float Collision::max(const float &p_A, const float &p_B, const float &p_C)
{
	return (p_A > p_B) ? (p_A > p_C) ? p_A : p_C : (p_B > p_C) ? p_B : p_C;
}

bool Collision::OBBVsPlane(OBB const &p_OBB, Plane const &p_Plane)
{
	float ex = p_OBB.getExtents().x;
	float ey = p_OBB.getExtents().y;
	float ez = p_OBB.getExtents().z;

	XMMATRIX bAxes = XMLoadFloat4x4(&p_OBB.getAxes());
	XMVECTOR pn = XMLoadFloat4(&p_Plane.normal);

	float r = ex * fabs(XMVectorGetX( XMVector4Dot(pn, bAxes.r[0]) )) +
			  ey * fabs(XMVectorGetX( XMVector4Dot(pn, bAxes.r[1]) )) +
			  ez * fabs(XMVectorGetX( XMVector4Dot(pn, bAxes.r[2]) ));

	XMVECTOR bc = XMLoadFloat4(&p_OBB.getPosition());
	float s = XMVectorGetX( XMVector4Dot(pn, bc) ) - p_Plane.d;

	return fabs(s) <= r;
}

DirectX::XMFLOAT4 Collision::findClosestPointOnTriangle(const DirectX::XMFLOAT4 &p_Point, const Vector4 &p_A, const Vector4 &p_B, const Vector4 &p_C)
{
	DirectX::XMVECTOR ab, ac, ap, a, b, c, pos;
	a = Vector4ToXMVECTOR(&p_A);
	b = Vector4ToXMVECTOR(&p_B);
	c = Vector4ToXMVECTOR(&p_C);
	pos = DirectX::XMLoadFloat4(&p_Point);

	using DirectX::operator-;
	using DirectX::operator*;
	using DirectX::operator+;
	ab = b - a;
	ac = c - a;
	ap = pos - a;

	float d1 = DirectX::XMVector4Dot(ab, ap).m128_f32[0];
	float d2 = DirectX::XMVector4Dot(ac, ap).m128_f32[0];
		
	DirectX::XMFLOAT4 ret;
	if(d1 <= 0.f && d2 <= 0.f)
	{
		DirectX::XMStoreFloat4(&ret, a);
		return ret;
	}

	DirectX::XMVECTOR bp = pos - b;
	float d3 = DirectX::XMVector4Dot(ab, bp).m128_f32[0];
	float d4 = DirectX::XMVector4Dot(ac, bp).m128_f32[0];

	if(d3 >= 0.f && d4 <= d3)
	{
		DirectX::XMStoreFloat4(&ret, b);
		return ret;
	}

	float vc = d1*d4 - d3*d2;
	if(vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
	{
		float v = d1 / (d1 - d3);
		DirectX::XMStoreFloat4(&ret, a + v * ab);
		return ret;
	}

	DirectX::XMVECTOR cp = pos - c;
	float d5 = DirectX::XMVector3Dot(ab, cp).m128_f32[0];
	float d6 = DirectX::XMVector3Dot(ac, cp).m128_f32[0];

	if(d6 >= 0.f && d5 <= d6)
	{
		DirectX::XMStoreFloat4(&ret, c);
		return ret;
	}

	float vb = d5*d2 - d1*d6;
	if(vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
	{
		float w = d2 / (d2 - d6);
		DirectX::XMStoreFloat4(&ret, a + w * ac);
		return ret;
	}

	float va = d3*d6 - d5*d4;
	if(va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		DirectX::XMStoreFloat4(&ret, b + w * ( c - b ));
		return ret;
	}

	float denom = 1.f / ( va + vb + vc);

	float v = vb * denom;
	float w = vc * denom;
	DirectX::XMStoreFloat4(&ret, a + ab * v + ac * w);
	return ret;
}

bool Collision::isZeroVector(XMVECTOR p_v)
{
	if(p_v.m128_f32[0] > EPSILON && p_v.m128_f32[0] < EPSILON)
		if(p_v.m128_f32[0] > EPSILON && p_v.m128_f32[0] < EPSILON)
			if(p_v.m128_f32[0] > EPSILON && p_v.m128_f32[0] < EPSILON)
				return true;
		

	return false;
}

// Test if AABB b intersects plane p
bool Collision::AABBVsPlane(AABB const &b, Plane const &p)
{
	// These two lines not necessary with a (center, extents) AABB representation
	XMVECTOR c = XMLoadFloat4(&b.getPosition()); // AABB center
	XMVECTOR e = XMLoadFloat4(&b.getHalfDiagonal());// b.max - c; // positive extents

	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	float r = e.m128_f32[0]*fabs(p.normal.x) + e.m128_f32[1]*fabs(p.normal.y) + e.m128_f32[2]*fabs(p.normal.z);
	// Compute distance of box center from plane
	float s = XMVector3Dot(XMLoadFloat4(&p.normal), c).m128_f32[0] - p.d;
	// Intersection occurs when distance s falls within [-r,+r] interval
	return fabs(s) <= r;
}