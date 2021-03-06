#include "EdgeCollisionResponse.h"
#include "ClientExceptions.h"
#include "Components.h"

using namespace DirectX;

EdgeCollisionResponse::EdgeCollisionResponse(void)
{
}

EdgeCollisionResponse::~EdgeCollisionResponse(void)
{
}

bool EdgeCollisionResponse::checkCollision(HitData &p_Hit, Vector3 p_EdgePosition, Vector3 p_EdgeOrientation, Player *p_Player)
{
	if(!p_Player->getForceMove() && p_Hit.collider == p_Player->getBody())
	{
		XMFLOAT3 collisionNormal = Vector4ToXMFLOAT3(&p_Hit.colNorm);

		Actor::ptr actor = p_Player->getActor().lock();

		bool isPlayerBody = false;

		if(actor)
		{
			for(unsigned i = 0; i < actor->getBodyHandles().size(); i++)
			{
				if(p_Player->getBody() == p_Hit.collisionVictim)
					isPlayerBody = true;
			}
		}

		if(!isPlayerBody)
			p_Player->setGroundNormal(collisionNormal);

		if (p_Hit.isEdge)
		{
			handleCollision(p_Player, p_EdgePosition, XMLoadFloat3(&collisionNormal),
				p_EdgeOrientation);
		}
		return true;
	}
	return false;
}

void EdgeCollisionResponse::handleCollision(Player *p_Player, Vector3 p_EdgePosition, XMVECTOR p_VictimNormal,
	Vector3 p_EdgeOrientation)
{
	XMFLOAT3 playerOrigPos = p_Player->getPosition();

	XMVECTOR vReachPointCenter = XMLoadFloat3(&playerOrigPos) - Vector3ToXMVECTOR(&p_EdgePosition, 0);

	XMVECTOR vEdgeOrientation = Vector3ToXMVECTOR(&p_EdgeOrientation, 0);
	vEdgeOrientation = XMVector3Normalize(vEdgeOrientation);

	vReachPointCenter = (XMVector3Dot(vReachPointCenter, vEdgeOrientation) * vEdgeOrientation) + Vector3ToXMVECTOR(&p_EdgePosition, 0);
	XMFLOAT3 nearestPoint;
	XMStoreFloat3(&nearestPoint, vReachPointCenter);

	XMVECTOR look = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	Actor::ptr actor = p_Player->getActor().lock();
	if(actor)
	{
		std::shared_ptr<LookInterface> lookComp = p_Player->getActor().lock()->getComponent<LookInterface>(LookInterface::m_ComponentId).lock();
		XMMATRIX rotation = XMLoadFloat4x4(&lookComp->getRotationMatrix());
		look = XMVector3Transform(look, rotation);
		look = XMVectorSet(look.m128_f32[0], 0.0f, look.m128_f32[2], 0.0f);
	}
	XMVECTOR victimNormalProjected = XMVectorSet(p_VictimNormal.m128_f32[0], 0.0f, p_VictimNormal.m128_f32[2], 0.0f);
	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(look, victimNormalProjected));

	if(angle >= 2.3625f && angle <= 3.9375f)
	{
		if(playerOrigPos.y < nearestPoint.y)
		{
			DirectX::XMFLOAT3 victimNormal;
			DirectX::XMStoreFloat3(&victimNormal, p_VictimNormal);

			if (playerOrigPos.y + p_Player->getHeight() * 0.1f > nearestPoint.y)
				return;
			else if(playerOrigPos.y  + p_Player->getKneeHeight() - p_Player->getHeight() * 0.1f > nearestPoint.y)
				p_Player->forceMove("Climb1", victimNormal, p_EdgePosition, p_EdgeOrientation);
			else if(playerOrigPos.y  + p_Player->getWaistHeight() > nearestPoint.y)
				p_Player->forceMove("Climb2", victimNormal, p_EdgePosition, p_EdgeOrientation);
			else if(playerOrigPos.y  + p_Player->getChestHeight() > nearestPoint.y)
				p_Player->forceMove("Climb3", victimNormal, p_EdgePosition, p_EdgeOrientation);
			else if(playerOrigPos.y  + p_Player->getHeight() > nearestPoint.y)
				p_Player->forceMove("Climb4", victimNormal, p_EdgePosition, p_EdgeOrientation);
			else{}
		}
	}
}