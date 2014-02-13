#pragma once
#include "Actor.h"
#include "IPhysics.h"

#include <DirectXMath.h>

class Player
{
private:
	IPhysics *m_Physics;
	std::weak_ptr<Actor> m_Actor;

	bool m_IsJumping;
	int m_JumpCount, m_JumpCountMax;
    float m_JumpTime, m_JumpTimeMax;
	float m_JumpForce;
	float m_MaxSpeed; // Centimeters per secound
	float m_AccConstant;
	float m_DirectionX;	// (-1 - +1)
	float m_DirectionZ;	// (-1 - +1)
	DirectX::XMFLOAT3 m_GroundNormal;

	bool m_ForceMove;
	float m_CurrentForceMoveTime;
	std::vector<DirectX::XMFLOAT2> m_ForceMoveY;
	std::vector<DirectX::XMFLOAT2> m_ForceMoveZ;
	DirectX::XMFLOAT4X4 m_ForceMoveRotation;
	DirectX::XMFLOAT3 m_ForceMoveStartPos;
	DirectX::XMFLOAT3 m_CenterReachPos;
	DirectX::XMFLOAT3 m_Side;
	DirectX::XMFLOAT3 m_EdgeOrientation;
	DirectX::XMFLOAT3 m_forward;
	std::string m_ClimbId;
	DirectX::XMFLOAT3 m_LastSafePosition;

	float m_Height; 
	float m_EyeHeight;

public:
	/**
	* Constructor
	*/
	Player(void);
	
	/**
	* Destructor
	*/
	~Player(void);
	
	/**
	* Initialize the player.
	* @param p_Physics a pointer to the physics engine
	* @param p_Actor the player actor
	*/
	void initialize(IPhysics *p_Physics, std::weak_ptr<Actor> p_Actor);
	
	/**
	* Updates the player's actions such as movement and jumping. If forced movement is active, the position will be updated between two stored positions.
	* @param p_DeltaTime the dt between two consecutive frames
	*/
	void update(float p_DeltaTime);

	/**
	* Forces the player to change its position. The player can not control the movement.
	* @param p_ClimbId the name of the current climb animation
	* @param p_CollisionNormal calculated in the physics
	* @param p_BoxPos the center position of the edge box
	* @param p_BoxOrientation the 2D orientation of the box
	*/
	virtual void forceMove(std::string p_ClimbId, DirectX::XMFLOAT3 p_CollisionNormal, DirectX::XMFLOAT3 p_BoxPos, DirectX::XMFLOAT3 p_EdgeOrientation);

	/**
	 * If the player is in a force move some IK groups might be locked onto points and need updating.
	 */
	void updateIKJoints();

	/**
	* Sets the position of the player at specified position in the game world.
	* @param p_Position the position where to place the player
	*/
	void setPosition(const DirectX::XMFLOAT3 &p_Position);
	
	/**
	* Gets the position of the player.
	* @return the position
	*/
	virtual DirectX::XMFLOAT3 getPosition(void) const;

	/**
	 * Get the eye position of the player.
	 *
	 * @return the position of the players eyes
	 */
	DirectX::XMFLOAT3 getEyePosition() const;

	/**
	 * Get the position that would be on the ground if the player would be standing on a flat surface.
	 *
	 * @return the ground position of the player
	 */
	DirectX::XMFLOAT3 getGroundPosition() const;

	/**
	 * Get the center position of the players collision volume.
	 *
	 * @return the collision volume center in cm, in world coordinates.
	 */
	DirectX::XMFLOAT3 getCollisionCenter() const;

	/**
	* Gets the height of the player.
	* @return the height
	*/
	float getHeight(void) const;
	float getChestHeight(void) const;
	float getWaistHeight(void) const;
	float getKneeHeight(void) const;
	
	/**
	 * Sets the player's height to a wanted value.
	 * @param p_Height, the wanted height
	 */
	void setHeight(float p_Height);
	
	/**
	* Gets the body handle of the player.
	* @return the body handle
	*/
	virtual BodyHandle getBody(void) const;

	/**
	* Gets if the player is currently forced to change position.
	* @return true = the player has no control of movement, false = the player is able to move by itself
	*/
	virtual bool getForceMove(void);

	/**
	 * Get the current velocity of the player.
	 *
	 * @return the velocity of the player in cm in world space
	 */
	Vector3 getVelocity() const;
	Vector3 getDirection() const;

	/**
	* Makes the player jump if not already jumping.
	*/
	void setJump(void);
	
	/**
	* Sets the player's x-direction.
	*/
	void setDirectionX(float p_DirectionX);
	
	/**
	* Sets the player's z-direction.
	*/
	void setDirectionZ(float p_DirectionZ);

	/**
	 * Get the actor that represents the player.
	 *
	 * @return an actor, or an empty pointer if none exists
	 */
	std::weak_ptr<Actor> getActor() const;
	/**
	 * Set a new actor to represent the player.
	 *
	 * @param p_Actor a valid actor, or an empty pointer to remove
	 */
	void setActor(std::weak_ptr<Actor> p_Actor);

	/**
	 * The player's ground normal. Ground Normal equals the perpendicular vector from the surface the player is standing on.
	 * 
	 * @return the normal.
	 */
	DirectX::XMFLOAT3 getGroundNormal() const;
	/**
	 * Sets the player's ground normal. Ground Normal equals the perpendicular vector from the surface the player is standing on.
	 *
	 * @param p_Normal the new ground normal.
	 */
	void setGroundNormal(DirectX::XMFLOAT3 p_Normal);

	/**
	 * Sets the player's respawn position,  which is used when the player goes out of bounds.
	 *
	 * @param p_Position the position where the player will spawn after going to far away
	 */
	void setSpawnPosition(Vector3 p_Position);

private:
	void jump(float dt);
	void move(float p_DeltaTime);
};