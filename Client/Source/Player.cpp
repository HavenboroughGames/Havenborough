#include "Player.h"

Player::Player(void)
{
	m_Physics = nullptr;
	m_JumpCount = 0;
    m_JumpCountMax = 2;
    m_JumpDelay = 0;
    m_JumpDelayMax = 0.1f;
    m_JumpTime = 0.f;
    m_JumpTimeMax = 0.15f;
	m_JumpForce = 6500.f;
	m_IsJumping = false;
	m_PrevForce = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	m_MaxSpeed = 1000.f;
	m_AccConstant = 250.f;
	m_DirectionZ = 0.f;
	m_DirectionX = 0.f;
	m_ForceMove = false;
	m_ForceMoveTime = 1.0f;
	m_ForceMoveSpeed = 1.f;
	m_CurrentForceMoveTime = 0.f;
}

Player::~Player(void)
{
	m_Physics = nullptr;
}

void Player::initialize(IPhysics *p_Physics, XMFLOAT3 p_Position, XMFLOAT3 p_LookDirection)
{
	m_Physics = p_Physics;
	m_Position = p_Position;
	m_LookDirection = p_LookDirection;
	m_TempHeight = 180.f;
	m_KneeHeight = 50.f;
	m_EyeHeight = 165.f;
	Vector3 kneePos(m_Position.x, m_Position.y + 90.f, m_Position.z);
	//m_PlayerBody = m_Physics->createOBB(68.f, false, kneePos, Vector3(50.f, 90.f, 50.f), false);
	m_PlayerBody = m_Physics->createSphere(68.f, false, kneePos, m_KneeHeight);

}

XMFLOAT3 Player::getPosition(void) const
{
	return m_Position;
}

XMFLOAT3 Player::getEyePosition() const
{
	XMFLOAT3 eyePosition = m_Position;
	eyePosition.y += m_EyeHeight;
	return eyePosition;
}

void Player::setPosition(const XMFLOAT3 &p_Position)
{
	m_Position = p_Position;
	m_Physics->setBodyPosition(m_PlayerBody, XMFLOAT3ToVector3(&m_Position));
}


XMFLOAT3 Player::getGroundPosition() const
{
	return m_Position;
}

XMFLOAT3 Player::getCollisionCenter() const
{
	return XMFLOAT3(m_Position.x, m_Position.y + m_KneeHeight, m_Position.z);
}

float Player::getHeight() const
{
	return m_TempHeight;
}

BodyHandle Player::getBody(void) const
{
	return m_PlayerBody;
}

void Player::setJump(void)
{
	  if(m_Physics->getBodyInAir(m_PlayerBody))
    {
            m_JumpCount++;
    }

    if(!m_IsJumping && m_JumpCount < m_JumpCountMax)
    {
            //m_JumpCount++;
            m_IsJumping = true;
            if(m_JumpCount > 0)
            {
                    Vector4 temp = m_Physics->getBodyVelocity(m_PlayerBody);
                    m_Physics->setBodyVelocity(m_PlayerBody, Vector3(temp.x, 0.f, temp.z));
            }
            m_Physics->applyForce(m_PlayerBody, Vector3(0.f, m_JumpForce, 0.f));
    }
}

void Player::setDirectionX(float p_DirectionX)
{
	m_DirectionX = p_DirectionX;
}

void Player::setDirectionZ(float p_DirectionZ)
{
	m_DirectionZ = p_DirectionZ;
}

bool Player::getForceMove(void)
{
	return m_ForceMove;
}

void Player::forceMove(Vector3 p_StartPosition, Vector3 p_EndPosition)
{
	if(!m_ForceMove)
	{
		m_ForceMove = true;
		m_ForceMoveStartPosition = p_StartPosition;
		m_ForceMoveEndPosition = p_EndPosition;
		m_Physics->setBodyVelocity(m_PlayerBody, Vector3(0,0,0));
	}
}

void Player::update(float p_DeltaTime)
{
	if(!m_ForceMove)
	{
		jump(p_DeltaTime);
		move();
	}
	else
	{
		float dt = m_CurrentForceMoveTime / m_ForceMoveTime;

		XMVECTOR startPos = XMLoadFloat3(&((XMFLOAT3)m_ForceMoveStartPosition));
		XMVECTOR endPos = XMLoadFloat3(&((XMFLOAT3)m_ForceMoveEndPosition));

		XMVECTOR currPosition = XMVectorLerp(startPos,
			endPos, dt);
		XMStoreFloat3(&m_Position, currPosition);

		Vector3 kneePos(m_Position.x, m_Position.y + m_KneeHeight, m_Position.z);
		m_Physics->setBodyPosition(m_PlayerBody, kneePos);


		m_CurrentForceMoveTime += p_DeltaTime * m_ForceMoveSpeed;
		if(m_CurrentForceMoveTime > m_ForceMoveTime)
		{
			m_ForceMove = false;
			m_CurrentForceMoveTime = 0.f;
		}
	}
}

Vector3 Player::getVelocity() const
{
	Vector4 vel4 = m_Physics->getBodyVelocity(m_PlayerBody);
	return Vector3(vel4.x, vel4.y, vel4.z);
}

std::weak_ptr<Actor> Player::getActor() const
{
	return m_Actor;
}

void Player::setActor(std::weak_ptr<Actor> p_Actor)
{
	m_Actor = p_Actor;
}

void Player::jump(float dt)
{
	if(m_IsJumping)
	{
		m_JumpTime += dt;
		if(m_JumpTime > m_JumpTimeMax)
		{
			m_Physics->applyForce(m_PlayerBody, Vector3(0.f, -m_JumpForce, 0.f));
			m_IsJumping = false;
			m_JumpTime = 0.f;
		}
	}
	if(!m_IsJumping && !m_Physics->getBodyInAir(m_PlayerBody))
    {
		m_JumpCount = 0;
    }
}

void Player::move()
{
	Vector4 velocity = m_Physics->getBodyVelocity(m_PlayerBody);
	XMFLOAT4 currentVelocity = Vector4ToXMFLOAT4(&velocity);	// cm/s
	currentVelocity.y = 0.f;
	XMFLOAT4 maxVelocity(-m_DirectionX * m_MaxSpeed, 0.f, -m_DirectionZ * m_MaxSpeed, 0.f);	// cm/s

	XMFLOAT4 diffVel = XMFLOAT4(0.f, 0.f, 0.f, 0.f);	// cm/s
	XMFLOAT4 force = XMFLOAT4(0.f, 0.f, 0.f, 0.f);		// kg * m/s^2

	diffVel.x = maxVelocity.x - currentVelocity.x;
	diffVel.y = maxVelocity.y - currentVelocity.y;
	diffVel.z = maxVelocity.z - currentVelocity.z;
	diffVel.w = 0.f;

	force.x = diffVel.x / 100.f * m_AccConstant;
	force.y = diffVel.y / 100.f * m_AccConstant;
	force.z = diffVel.z / 100.f * m_AccConstant;
	force.w = 0.f;
	XMFLOAT3 forceDiff = XMFLOAT3(force.x - m_PrevForce.x, 0.f, force.z - m_PrevForce.z);	// kg * m/s^2
	m_PrevForce = force;

	m_Physics->applyForce(m_PlayerBody, XMFLOAT3ToVector3(&forceDiff));

	m_DirectionX = m_DirectionZ = 0.f;

	Vector4 kneePosition = m_Physics->getBodyPosition(m_PlayerBody);	// cm
	m_Position.x = kneePosition.x;
	m_Position.y = kneePosition.y - m_KneeHeight;
	m_Position.z = kneePosition.z;
}