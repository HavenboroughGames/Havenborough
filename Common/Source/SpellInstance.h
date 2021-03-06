#pragma once
#include "SpellDefinition.h"
#include "IPhysics.h"

#include <DirectXMath.h>
#include <vector>

class SpellInstance
{
public:

	/**
	 * 
	 */
	typedef std::shared_ptr<SpellInstance> ptr;

private:
	SpellDefinition::ptr m_SpellDefinition;

	float m_TimeLived; //in sec
	bool m_Collision;
	bool m_IsDead;
	bool m_IsColliding;
	Vector3 m_Velocity;

public:
	
	/**
	 * Basic constructor and deconstructor
	 */
	SpellInstance();
	~SpellInstance();
	
	/**
	 * Function to initialize a spell instance
	 * 
	 * @param p_SpellDefinition save the pointer so the instance know what definition to read from
	 * @param p_Direction give a direction to shoot the spell in
	 */
	void init(SpellDefinition::ptr p_SpellDefinition, Vector3 p_Direction);
	
	/**
	 * Function that will update the life timer and effect timer for the spell instance.
	 * 
	 * @param p_DeltaTime time interval between the two last frames
	 */
	void update(float p_DeltaTime);
	
	/**
	 * called when a collision had happen.
	 */
	void collisionHappened();
	/**
	 * Called to get the size of spell projectile.
	 * 
	 * @return the size of the spell. 
	 */
	Vector3 getSize() const;
	/**
	 * Called to get the radius on the spell sphere explosion.
	 * 
	 * @returning the exploding radius
	 */
	float getRadius() const;
	
	/**
	 * Called to get the velocity of the spell when flying in the air.
	 * 
	 * @return a vector3 containing the velocity of the spell instance
	 */
	Vector3 getVelocity() const;
	
	/**
	 * Called to get if the spell have collided.
	 * 
	 * @return the bool telling if there has been a collision (true = yes, false = no)
	 */
	bool hasCollided() const;
	
	/**
	 * Called to know if the actor is active or not.
	 * 
	 * @return the bool telling if the actor should be taken away or not
	 */
	bool isDead() const;
	
	/**
	 * Called when a spell times out or collide to see what path to take based on what type of spell.
	 * 
	 * @param p_DeltaTime time interval between the two last frames
	 * @param p_Physics a pointer to the physics
	 * @param p_Hit all the hitdata about the colliding bodyhandle
	 */
	void spellHit(IPhysics* p_Physics, const HitData& p_Hit, BodyHandle p_CasterBody);

	/**
	 * Called to see the status of the m_IsColliding bool.
	 * 
	 * @return a bool status, true if a collision happen else false
	 */
	bool isColliding() const;

private:
	void explodeSpell(IPhysics* p_Physics, const HitData& p_Hit, BodyHandle p_CasterBody);

};

