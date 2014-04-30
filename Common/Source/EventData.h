#pragma once
#include "IEventData.h"

#include "Actor.h"
#include "AnimationData.h"
#include "LightClass.h"
#include "Utilities/XMFloatUtil.h"

#pragma warning(push)
#pragma warning(disable : 4100)

#pragma region EXAMPLE READ THIS IF YOU DO NOT KNOW HOW TO CREATE AN EVENT
//////////////////////////////////////////////////////////////////////////
/// EXAMPLE EVENT DATA AND USED FOR TESTING
//////////////////////////////////////////////////////////////////////////
class TestEventData : public BaseEventData
{
private: 
	//Parameter for event, can be of any number and type
	bool m_AssumingDirectControl;

public:
	/**
	* Unique identifier for event data type. This one is an example and for testing.
	* E.g. IEventData::Type UniqueEventDataName::sk_EventType(unique_hex);
	*/
	static const IEventData::Type sk_EventType = Type(0x77dd2b3a);
	
	explicit TestEventData(bool p_AssumingControl) :
		m_AssumingDirectControl(p_AssumingControl)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new TestEventData(m_AssumingDirectControl));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		p_Out << m_AssumingDirectControl;
	}

	virtual const char *getName(void) const override
	{
		return "TestEvent";
	}

	/**
	* Used to get the event data. User defined function.
	* Can be of any number of functions and have any return type.
	*/
	bool directInterventionIsNecessary(void) const
	{
		return m_AssumingDirectControl;
	}
};

#pragma endregion

class LightEventData : public BaseEventData
{
private:
	LightClass m_Light;

public:
	static const Type sk_EventType = Type(0x748d2b5a);
	
	explicit LightEventData(LightClass p_Light) :
		m_Light(p_Light)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new LightEventData(m_Light));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "LightEvent";
	}

	LightClass getLight(void) const
	{
		return m_Light;
	}
};

class RemoveLightEventData : public BaseEventData
{
private:
	LightClass::Id m_Id;

public:
	static const Type sk_EventType = Type(0x128d2b5a);
	
	explicit RemoveLightEventData(LightClass::Id p_Id) :
		m_Id(p_Id)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new RemoveLightEventData(m_Id));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "RemoveLightEvent";
	}

	LightClass::Id getId() const
	{
		return m_Id;
	}
};

class CreateMeshEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	std::string m_MeshName;
	Vector3 m_Scale;
	Vector3 m_ColorTone;
	std::string m_Style;

public:
	static const Type sk_EventType = Type(0xdeadbeef);

	CreateMeshEventData(unsigned int p_Id, const std::string& p_MeshName,
		Vector3 p_Scale, Vector3 p_ColorTone, const std::string& p_Style)
		:	m_Id(p_Id),
			m_MeshName(p_MeshName),
			m_Scale(p_Scale),
			m_ColorTone(p_ColorTone),
			m_Style(p_Style)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new CreateMeshEventData(m_Id, m_MeshName, m_Scale, m_ColorTone, m_Style));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "CreateMeshEvent";
	}

	std::string getMeshName() const
	{
		return m_MeshName;
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getScale() const
	{
		return m_Scale;
	}

	Vector3 getColorTone() const
	{
		return m_ColorTone;
	}

	std::string getStyle() const
	{
		return m_Style;
	}
};

class RemoveMeshEventData : public BaseEventData
{
private:
	unsigned int m_Id;

public:
	static const Type sk_EventType = Type(0xdeadebbe);

	RemoveMeshEventData(unsigned int p_Id)
		:	m_Id(p_Id)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new RemoveMeshEventData(m_Id));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "RemoveMeshEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}
};

class UpdateModelPositionEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0x77dd2b5b);

	UpdateModelPositionEventData(unsigned int p_Id, Vector3 p_Position)
		:	m_Id(p_Id), m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateModelPositionEventData(m_Id, m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateModelPositionEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}
};

class UpdateModelScaleEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Scale;

public:
	static const Type sk_EventType = Type(0x77dd2b5c);

	UpdateModelScaleEventData(unsigned int p_Id, Vector3 p_Scale)
		:	m_Id(p_Id),m_Scale(p_Scale)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateModelScaleEventData(m_Id, m_Scale));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateModelScaleEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getScale() const
	{
		return m_Scale;
	}
};

class UpdateModelRotationEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Rotation;

public:
	static const Type sk_EventType = Type(0x77dd2b5d);

	UpdateModelRotationEventData(unsigned int p_Id, Vector3 p_Rotation)
		:	m_Id(p_Id), m_Rotation(p_Rotation)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateModelRotationEventData(m_Id, m_Rotation));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateModelRotationEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getRotation() const
	{
		return m_Rotation;
	}
};

class UpdateAnimationEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	const std::vector<DirectX::XMFLOAT4X4>& m_AnimationData;
	const AnimationData::ptr m_Animation;
	const DirectX::XMFLOAT4X4 m_World;

public:
	static const Type sk_EventType = Type(0x14dd2b5d);

	UpdateAnimationEventData(unsigned int p_Id, const std::vector<DirectX::XMFLOAT4X4>& p_AnimationData, AnimationData::ptr p_Animation,
		DirectX::XMFLOAT4X4 p_World)
		:	m_Id(p_Id), m_AnimationData(p_AnimationData), m_Animation(p_Animation), m_World(p_World)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateAnimationEventData(m_Id, m_AnimationData, m_Animation, m_World));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateAnimationEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	const std::vector<DirectX::XMFLOAT4X4>& getAnimationData() const
	{
		return m_AnimationData;
	}

	const AnimationData::ptr getAnimation() const
	{
		return m_Animation;
	}

	const DirectX::XMFLOAT4X4 getWorld() const
	{
		return m_World;
	}
};

class GameStartedEventData : public BaseEventData
{
private:

public:
	static const Type sk_EventType = Type(0x38ae3f31);

	GameStartedEventData()
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new GameStartedEventData);
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "GameStartedEvent";
	}
};

class QuitGameEventData : public BaseEventData
{
private:

public:
	static const Type sk_EventType = Type(0x846e56eb);

	QuitGameEventData()
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new QuitGameEventData);
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "QuitGameEvent";
	}
};

class MouseEventDataLock : public BaseEventData
{
private: 
	//Parameter for event, can be of any number and type
	bool m_State;

public:
	/**
	* Unique identifier for event data type. This one is an example and for testing.
	* E.g. IEventData::Type UniqueEventDataName::sk_EventType(unique_hex);
	*/
	static const IEventData::Type sk_EventType = Type(0x72dd2b3a);
	
	explicit MouseEventDataLock(bool p_LockState) :
		m_State(p_LockState)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new MouseEventDataLock(m_State));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		p_Out << m_State;
	}

	virtual const char *getName(void) const override
	{
		return "MouseEventDataLock";
	}

	/**
	* Used to get the event data. User defined function.
	* Can be of any number of functions and have any return type.
	*/
	bool getLockState(void) const
	{
		return m_State;
	}
};

class MouseEventDataShow : public BaseEventData
{
private: 
	bool m_State;

public:
	static const IEventData::Type sk_EventType = Type(0x22dd2b3a);
	
	explicit MouseEventDataShow(bool p_HideState) :
		m_State(p_HideState)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new MouseEventDataShow(m_State));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		p_Out << m_State;
	}

	virtual const char *getName(void) const override
	{
		return "MouseEventDataShow";
	}

	bool getShowState(void) const
	{
		return m_State;
	}
};

class MouseEventDataPie : public BaseEventData
{
private: 
	Vector2 m_MousePos;
	bool m_PieStatus;

public:
	static const IEventData::Type sk_EventType = Type(0xa5cc161);
	
	explicit MouseEventDataPie(Vector2 p_MousePos, bool p_PieStatus) :
	m_MousePos(p_MousePos), m_PieStatus(p_PieStatus)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new MouseEventDataPie(m_MousePos, m_PieStatus));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		//p_Out << m_MousePos;
	}

	virtual const char *getName(void) const override
	{
		return "MouseEventDataPie";
	}

	Vector2 getMousePos(void) const
	{
		return m_MousePos;
	}

	bool getPieStatus() const
	{
		return m_PieStatus;
	}
};

class ChangeColorToneEvent : public BaseEventData
{
private: 
	Vector3 m_ColorTone;
	unsigned int m_MeshId;

public:
	static const IEventData::Type sk_EventType = Type(0xbabbab3a);
	
	explicit ChangeColorToneEvent(unsigned int p_MeshId, Vector3 p_ColorTone) :
		m_MeshId(p_MeshId),
		m_ColorTone(p_ColorTone)
	{
	}

	virtual const IEventData::Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new ChangeColorToneEvent(m_MeshId, m_ColorTone));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
		//p_Out << m_ColorTone;
	}

	virtual const char *getName(void) const override
	{
		return "ChangeColorToneEvent";
	}

	Vector3 getColorTone(void) const
	{
		return m_ColorTone;
	}

	unsigned int getMeshId(void) const
	{
		return m_MeshId;
	}
};

class CreateParticleEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	std::string m_EffectName;
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0x54456edb);

	CreateParticleEventData(unsigned int p_Id, const std::string& p_EffectName, Vector3 p_Position)
		:	m_Id(p_Id),
			m_EffectName(p_EffectName),
			m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new CreateParticleEventData(m_Id, m_EffectName, m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "CreateParticleEvent";
	}

	std::string getEffectName() const
	{
		return m_EffectName;
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}
};

class RemoveParticleEventData : public BaseEventData
{
private:
	unsigned int m_Id;

public:
	static const Type sk_EventType = Type(0x82544aeb);

	RemoveParticleEventData(unsigned int p_Id)
		:	m_Id(p_Id)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new RemoveParticleEventData(m_Id));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "RemoveParticleEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}
};

class UpdateParticlePositionEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0xd02a90fc);

	UpdateParticlePositionEventData(unsigned int p_Id, Vector3 p_Position)
		:	m_Id(p_Id),
			m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateParticlePositionEventData(m_Id, m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateParticlePositionEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}
};

class UpdateParticleRotationEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Rotation;

public:
	static const Type sk_EventType = Type(0xe738ee21);

	UpdateParticleRotationEventData(unsigned int p_Id, Vector3 p_Rotation)
		:	m_Id(p_Id),
		m_Rotation(p_Rotation)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateParticleRotationEventData(m_Id, m_Rotation));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateParticleRotationEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector3 getRotation() const
	{
		return m_Rotation;
	}
};


class UpdateParticleBaseColorEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector4 m_BaseColor;

public:
	static const Type sk_EventType = Type(0xdab6df88);

	UpdateParticleBaseColorEventData(unsigned int p_Id, Vector4 p_BaseColor)
		:	m_Id(p_Id),
		m_BaseColor(p_BaseColor)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateParticleBaseColorEventData(m_Id, m_BaseColor));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateParticleBaseColorEvent";
	}

	unsigned int getId() const
	{
		return m_Id;
	}

	Vector4 getBaseColor() const
	{
		return m_BaseColor;
	}
};

class SpellHitEventData : public BaseEventData
{
private:
	Actor m_SpellActor;
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0xca743787);

	SpellHitEventData(Actor p_SpellActor, Vector3 p_Position)
		:	m_SpellActor(p_SpellActor),
			m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new SpellHitEventData(m_SpellActor, m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "SpellHitEventData";
	}

	Actor getSpellActor() const
	{
		return m_SpellActor;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}
};

class SpellHitSphereEventData : public BaseEventData
{
private:
	BodyHandle m_CollisionVictim;

public:
	static const Type sk_EventType = Type(0x8d03c1df);

	SpellHitSphereEventData(BodyHandle p_CollisionVictim)
		:	m_CollisionVictim(p_CollisionVictim)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new SpellHitSphereEventData(m_CollisionVictim));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "SpellHitSphereEventData";
	}

	BodyHandle getCollisionVictim() const
	{
		return m_CollisionVictim;
	}
};

class PlayerIsHitBySpellEventData : public BaseEventData
{
private:

public:
	static const Type sk_EventType = Type(0x4540b51a);

	PlayerIsHitBySpellEventData()
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new PlayerIsHitBySpellEventData());
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "PlayerIsHitBySpellEventData";
	}
};

class RemoveActorEventData : public BaseEventData
{
private:
	Actor::Id m_Actor;

public:
	static const Type sk_EventType = Type(0x91615dff);

	RemoveActorEventData(Actor::Id p_Actor)
		:	m_Actor(p_Actor)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new RemoveActorEventData(m_Actor));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "RemoveActorEvent";
	}

	unsigned int getActorId() const
	{
		return m_Actor;
	}
};

class UpdateGraphicalCountdownEventData : public BaseEventData
{
private:
	std::wstring m_Text;
	Vector4 m_Color;
	Vector3 m_Scale;

public:
	static const Type sk_EventType = Type(0x01015dff);

	UpdateGraphicalCountdownEventData(std::wstring p_Text, Vector4 p_Color, Vector3 p_Scale)
		:	m_Text(p_Text), m_Color(p_Color), m_Scale(p_Scale)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateGraphicalCountdownEventData(m_Text, m_Color, m_Scale));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateGraphicalCountdownEventData";
	}

	std::wstring getText() const
	{
		return m_Text;
	}
	Vector4 getColor() const
	{
		return m_Color;
	}
	Vector3 getScale() const
	{
		return m_Scale;
	}
};

class UpdateGraphicalManabarEventData : public BaseEventData
{
private:
	float m_CurrentMana;
	float m_PreviousMana;
	float m_ManaCost;

public:
	static const Type sk_EventType = Type(0x51515dff);

	UpdateGraphicalManabarEventData(float p_CurrentMana, float p_PrevioudMana, float p_ManaCost)
		:	m_CurrentMana(p_CurrentMana), m_PreviousMana(p_PrevioudMana), m_ManaCost(p_ManaCost)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateGraphicalManabarEventData(m_CurrentMana, m_PreviousMana, m_ManaCost));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateGraphicalManabarEventData";
	}

	float getManaCost() const
	{
		return m_ManaCost;
	}

	float getCurrentMana() const
	{
		return m_CurrentMana;
	}

	float getPreviousMana() const
	{
		return m_PreviousMana;
	}

	float getDiffCurrPrevious() const
	{
		return m_CurrentMana - m_PreviousMana;
	}
};

class UpdateCheckpointPositionEventData : public BaseEventData
{
private:
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0x59515dff);

	UpdateCheckpointPositionEventData(Vector3 p_Position)
		:	m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateCheckpointPositionEventData(m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateCheckpointPositionEventData";
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}
};

class UpdatePlayerTimeEventData : public BaseEventData
{
private:
	float m_Time;

public:
	static const Type sk_EventType = Type(0x6d441ab5);

	UpdatePlayerTimeEventData(float p_Time)
		:	m_Time(p_Time)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdatePlayerTimeEventData(m_Time));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdatePlayerTimeEventData";
	}

	float getTime() const
	{
		return m_Time;
	}
};

class UpdatePlayerElapsedTimeEventData : public BaseEventData
{
private:
	float m_ElapsedTime;

public:
	static const Type sk_EventType = Type(0x4c1c642c);

	UpdatePlayerElapsedTimeEventData(float p_ElapsedTime)
		:	m_ElapsedTime(p_ElapsedTime)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdatePlayerElapsedTimeEventData(m_ElapsedTime));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdatePlayerElapsedTimeEventData";
	}

	float getElapsedTime() const
	{
		return m_ElapsedTime;
	}
};

class UpdatePlayerRaceEventData : public BaseEventData
{
private:
	int m_Position;

public:
	static const Type sk_EventType = Type(0x3cd9fd2b);

	UpdatePlayerRaceEventData(int p_Position)
		:	m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdatePlayerRaceEventData(m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdatePlayerRaceEventData";
	}

	int getPosition() const
	{
		return m_Position;
	}
};

class activateHUDEventData : public BaseEventData
{
private:
	bool m_State;

public:
	static const Type sk_EventType = Type(0x7cd2fd2b);

	activateHUDEventData(bool p_State)
		:	m_State(p_State)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new activateHUDEventData(m_State));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdatePlayerRaceEventData";
	}

	bool getState() const
	{
		return m_State;
	}
};

class GetNrOfCheckpoints : public BaseEventData
{
private:
	int m_NrOfCheckpoints;

public:
	static const Type sk_EventType = Type(0x381c71c3);

	GetNrOfCheckpoints(unsigned int p_NrOfCheckpoints)
		:	m_NrOfCheckpoints(p_NrOfCheckpoints)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new GetNrOfCheckpoints(m_NrOfCheckpoints));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateNrOfCheckpoints";
	}

	int getNumberOfCheckpoints() const
	{
		return m_NrOfCheckpoints;
	}
};

class UpdateTakenCheckpoints : public BaseEventData
{
private:
	int m_TakenCheckpoints;

public:
	static const Type sk_EventType = Type(0xdc022c82);

	UpdateTakenCheckpoints(unsigned int p_TakenCheckpoints)
		:	m_TakenCheckpoints(p_TakenCheckpoints)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new UpdateTakenCheckpoints(m_TakenCheckpoints));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "UpdateTakenCheckpoints";
	}

	int getNumberOfCheckpointsTaken() const
	{
		return m_TakenCheckpoints;
	}
};

class createWorldTextEventData : public BaseEventData
{
private:
	std::string m_Text;
	std::string m_Font;
	float m_FontSize;
	Vector4 m_FontColor;
	Vector4 m_BackgroundColor;
	Vector3 m_Position;
	float m_Scale;
	float m_Rotation;

	unsigned int m_ComponentId;

public:
	static const Type sk_EventType = Type(0x7cd2bbbb);

	createWorldTextEventData(std::string p_Text, std::string p_Font, float p_FontSize, Vector4 p_FontColor, 
		Vector4 p_BackgroundColor, Vector3 p_Position, float p_scale, float p_Rotation, unsigned int p_ComponentId)
		:	m_Text(p_Text), m_Font(p_Font), m_FontSize(p_FontSize), m_FontColor(p_FontColor), 
		m_BackgroundColor(p_BackgroundColor), m_Position(p_Position), m_Scale(p_scale), m_Rotation(p_Rotation), 
		m_ComponentId(p_ComponentId)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new createWorldTextEventData(m_Text, m_Font, m_FontSize, m_FontColor, m_BackgroundColor, m_Position,
			 m_Scale, m_Rotation, m_ComponentId));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "createWorldTextEventData";
	}

	std::wstring getText()
	{
		return std::wstring(m_Text.begin(), m_Text.end());
	}

	std::string getFont()
	{
		return m_Font;
	}

	float getFontSize()
	{
		return m_FontSize;
	}

	Vector4 getFontColor()
	{
		return m_FontColor;
	}

	Vector4 getBackgroundColor()
	{
		return m_BackgroundColor;
	}

	Vector3 getPosition()
	{
		return m_Position;
	}

	float getScale()
	{
		return m_Scale;
	}

	float getRotation()
	{
		return m_Rotation;
	}

	unsigned int getComponentId()
	{
		return m_ComponentId;
	}
};

class removeWorldTextEventData : public BaseEventData
{
private:
	unsigned int m_Id;

public:
	static const Type sk_EventType = Type(0x7cd2abcc);

	removeWorldTextEventData(unsigned int p_Id)
		:	m_Id(p_Id)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new removeWorldTextEventData(m_Id));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "removeWorldTextEventData";
	}

	unsigned int getId()
	{
		return m_Id;
	}
};

class updateWorldTextPositionEventData : public BaseEventData
{
private:
	unsigned int m_Id;
	Vector3 m_Position;

public:
	static const Type sk_EventType = Type(0x7cd2ccab);

	updateWorldTextPositionEventData(unsigned int p_Id, Vector3 p_Position)
		:	m_Id(p_Id), m_Position(p_Position)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new updateWorldTextPositionEventData(m_Id, m_Position));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "updateWorldTextPositionEventData";
	}

	unsigned int getId()
	{
		return m_Id;
	}

	Vector3 getPosition()
	{
		return m_Position;
	}
};

class FinishRaceEventData : public BaseEventData
{
public:
	typedef std::vector<std::pair<std::string, float>> GoalList;

private:
	const GoalList m_GoalList;

public:
	static const Type sk_EventType = Type(0x552dd5cc);

	FinishRaceEventData(const GoalList& p_GoalList)
		: m_GoalList(p_GoalList)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new FinishRaceEventData(m_GoalList));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "FinishRaceEvent";
	}

	const GoalList& getGoalList() const
	{
		return m_GoalList;
	}
};

class Create3DSoundEventData : public BaseEventData
{
private:
	float m_MinDistance;
	std::string m_SoundTitle;
	Actor::Id m_ActorID; 
	int m_SoundID;
	bool m_3D;
	bool m_Loop;
public:
	static const Type sk_EventType = Type(0xa05bbdd8);

	Create3DSoundEventData(const char* p_SoundTitle, Actor::Id p_ActorID, float p_MinDistance, int p_SoundID, bool p_3D, bool p_Loop)
		: m_SoundTitle(p_SoundTitle), m_ActorID(p_ActorID), m_MinDistance(p_MinDistance), m_SoundID(p_SoundID), m_3D(p_3D), m_Loop(p_Loop)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new Create3DSoundEventData(m_SoundTitle.c_str(), m_ActorID, m_MinDistance, m_SoundID, m_3D, m_Loop));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "Create3DSoundEventData";
	}

	const Actor::Id getActorID() const
	{
		return m_ActorID;
	}

	const std::string getSoundTitle() const
	{
		return m_SoundTitle;
	}

	const float getMinDistance() const
	{
		return m_MinDistance;
	}

	const int getSoundID() const
	{
		return m_SoundID;
	}

	const bool get3DBool() const
	{
		return m_3D;
	}

	const bool getLoopBool() const
	{
		return m_Loop;
	}
};

class Play3DSoundEventData : public BaseEventData
{
private:
	Actor::Id m_ActorID;
	int m_SoundID;
	Vector3 m_Position;
	Vector3 m_Velocity;

public:
	static const Type sk_EventType = Type(0x5fcc08af);

	Play3DSoundEventData(Actor::Id p_ActorID, int p_SoundID, Vector3 p_Position, Vector3 p_Velocity)
		: m_ActorID(p_ActorID), m_SoundID(p_SoundID), m_Position(p_Position), m_Velocity(p_Velocity)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new Play3DSoundEventData(m_ActorID, m_SoundID, m_Position, m_Velocity));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "Play3DSoundEventData";
	}

	const Actor::Id getActorID() const
	{
		return m_ActorID;
	}

	const int getSoundID() const
	{
		return m_SoundID;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}

	Vector3 getVelocity() const
	{
		return m_Velocity;
	}
};

class Release3DSoundEventData : public BaseEventData
{
private:
	Actor::Id m_ActorID;
	int m_SoundID;

public:
	static const Type sk_EventType = Type(0x32376336);

	Release3DSoundEventData(Actor::Id p_ActorID, int p_SoundID)
		: m_ActorID(p_ActorID), m_SoundID(p_SoundID)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new Release3DSoundEventData(m_ActorID, m_SoundID));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "Release3DSoundEventData";
	}

	const Actor::Id getActorID() const
	{
		return m_ActorID;
	}

	const int getSoundID() const
	{
		return m_SoundID;
	}
};

class Update3DSoundEventData : public BaseEventData
{
private:
	Actor::Id m_ActorID;
	int m_SoundID;
	Vector3 m_Position;
	Vector3 m_Velocity;
public:
	static const Type sk_EventType = Type(0x8941b8d4);

	Update3DSoundEventData(Actor::Id p_ActorID, int p_SoundID, Vector3 p_Position, Vector3 p_Velocity)
		: m_ActorID(p_ActorID), m_SoundID(p_SoundID), m_Position(p_Position), m_Velocity(p_Velocity)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new Update3DSoundEventData(m_ActorID, m_SoundID, m_Position, m_Velocity));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "Update3DSoundEventData";
	}

	const Actor::Id getActorID() const
	{
		return m_ActorID;
	}

	const int getSoundID() const
	{
		return m_SoundID;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}

	Vector3 getVelocity() const
	{
		return m_Velocity;
	}
};

class PausedSoundEventData : public BaseEventData
{
private:
	Actor::Id m_ActorID;
	int m_SoundID;
	bool m_Paused;
public:
	static const Type sk_EventType = Type(0xb76661e6);

	PausedSoundEventData(Actor::Id p_ActorID, int p_SoundID, bool p_Paused)
		: m_ActorID(p_ActorID), m_SoundID(p_SoundID), m_Paused(p_Paused)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new PausedSoundEventData(m_ActorID, m_SoundID, m_Paused));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "PausedSoundEventData";
	}

	const Actor::Id getActorID() const
	{
		return m_ActorID;
	}

	const int getSoundID() const
	{
		return m_SoundID;
	}

	const bool getPaused() const
	{
		return m_Paused;
	}
};

class CreateSingleSoundEventData : public BaseEventData
{
private:
	std::string m_SoundTitle;
	float m_MinDistance;
	Vector3 m_Position;
	Vector3 m_Velocity;
public:
	static const Type sk_EventType = Type(0xf169ae09);

	CreateSingleSoundEventData(std::string p_SoundTitle, float p_MinDistance, Vector3 p_Position, Vector3 p_Velocity)
		: m_SoundTitle(p_SoundTitle), m_MinDistance(p_MinDistance), m_Position(p_Position), m_Velocity(p_Velocity)
	{
	}

	virtual const Type &getEventType(void) const override
	{
		return sk_EventType;
	}

	virtual Ptr copy(void) const override
	{
		return Ptr(new CreateSingleSoundEventData(m_SoundTitle, m_MinDistance, m_Position, m_Velocity));
	}

	virtual void serialize(std::ostream &p_Out) const override
	{
	}

	virtual const char *getName(void) const override
	{
		return "CreateSingleSoundEventData";
	}

	const std::string getSoundTitle() const
	{
		return m_SoundTitle;
	}

	const float getMinDistance() const
	{
		return m_MinDistance;
	}

	Vector3 getPosition() const
	{
		return m_Position;
	}

	Vector3 getVelocity() const
	{
		return m_Velocity;
	}
};
#pragma warning(pop)