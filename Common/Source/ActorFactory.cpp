#include "ActorFactory.h"
#include "CommonExceptions.h"
#include "Components.h"
#include "SoundComponent.h"
#include "FlyingControlComponent.h"
#include "SplineControlComponent.h"
#include "HumanAnimationComponent.h"
#include "LookComponent.h"
#include "RunControlComponent.h"
#include "SpellComponent.h"
#include "PlayerBodyComponent.h"
#include "XMLHelper.h"

ActorFactory::ActorFactory(unsigned int p_BaseActorId)
	:	m_LastActorId(p_BaseActorId),
		m_LastModelComponentId(0),
		m_LastLightComponentId(0),
		m_LastParticleComponentId(0),
		m_LastSpellComponentId(0),
		m_LastTextComponentId(0),
		m_Physics(nullptr),
		m_SpellFactory(nullptr)
{
	m_ComponentCreators["PlayerPhysics"] = std::bind(&ActorFactory::createPlayerComponent, this);
	m_ComponentCreators["OBBPhysics"] = std::bind(&ActorFactory::createOBBComponent, this);
	m_ComponentCreators["AABBPhysics"] = std::bind(&ActorFactory::createAABBComponent, this);
	m_ComponentCreators["SpherePhysics"] = std::bind(&ActorFactory::createCollisionSphereComponent, this);
	m_ComponentCreators["MeshPhysics"] = std::bind(&ActorFactory::createBoundingMeshComponent, this);
	m_ComponentCreators["Model"] = std::bind(&ActorFactory::createModelComponent, this);
	m_ComponentCreators["ModelSinOffset"] = std::bind(&ActorFactory::createModelSinOffsetComponent, this);
	m_ComponentCreators["Movement"] = std::bind(&ActorFactory::createMovementComponent, this);
	m_ComponentCreators["CircleMovement"] = std::bind(&ActorFactory::createCircleMovementComponent, this);
	m_ComponentCreators["Pulse"] = std::bind(&ActorFactory::createPulseComponent, this);
	m_ComponentCreators["Sound"] = std::bind(&ActorFactory::createSoundComponent, this);
	m_ComponentCreators["Light"] = std::bind(&ActorFactory::createLightComponent, this);
	m_ComponentCreators["Particle"] = std::bind(&ActorFactory::createParticleComponent, this);
	m_ComponentCreators["Spell"] = std::bind(&ActorFactory::createSpellComponent, this);
	m_ComponentCreators["Look"] = std::bind(&ActorFactory::createLookComponent, this);
	m_ComponentCreators["HumanAnimation"] = std::bind(&ActorFactory::createHumanAnimationComponent, this);
	m_ComponentCreators["FlyingControl"] = std::bind(&ActorFactory::createFlyingControlComponent, this);
	m_ComponentCreators["SplineControl"] = std::bind(&ActorFactory::createSplineControlComponent, this);
	m_ComponentCreators["RunControl"] = std::bind(&ActorFactory::createRunControlComponent, this);
	m_ComponentCreators["TextComponent"] = std::bind(&ActorFactory::createTextComponent, this);
}

void ActorFactory::setPhysics(IPhysics* p_Physics)
{
	m_Physics = p_Physics;
}

void ActorFactory::setEventManager(EventManager* p_EventManager)
{
	m_EventManager = p_EventManager;
}

void ActorFactory::setResourceManager(ResourceManager* p_ResourceManager)
{
	m_ResourceManager = p_ResourceManager;
}

void ActorFactory::setAnimationLoader(AnimationLoader* p_AnimationLoader)
{
	m_AnimationLoader = p_AnimationLoader;
}

void ActorFactory::setSpellFactory(SpellFactory* p_SpellFactory)
{
	m_SpellFactory = p_SpellFactory;
}

SpellFactory* ActorFactory::getSpellFactory()
{
	return m_SpellFactory;
}

void ActorFactory::setActorList(std::weak_ptr<ActorList> p_ActorList)
{
	m_ActorList = p_ActorList;
}

Actor::ptr ActorFactory::createActor(const tinyxml2::XMLElement* p_Data)
{
	return createActor(p_Data, getNextActorId());
}

Actor::ptr ActorFactory::createActor(const tinyxml2::XMLElement* p_Data, Actor::Id p_Id)
{
	Actor::ptr actor(new Actor(p_Id, m_EventManager, m_ActorList));
	actor->initialize(p_Data);

	for (const tinyxml2::XMLElement* node = p_Data->FirstChildElement(); node; node = node->NextSiblingElement())
	{
		ActorComponent::ptr component(createComponent(node));
		if (component)
		{
			actor->addComponent(component);
			component->setOwner(actor.get());
		}
		else
		{
			return Actor::ptr();
		}
	}

	actor->postInit();

	return actor;
}

void addEdge(tinyxml2::XMLPrinter& p_Printer, Vector3 p_Position, Vector3 p_Halfsize)
{
	p_Printer.OpenElement("AABBPhysics");
	p_Printer.PushAttribute("IsEdge", true);
	pushVector(p_Printer, "Halfsize", p_Halfsize);
	pushVector(p_Printer, "OffsetPosition", p_Position);
	p_Printer.CloseElement();
}

Actor::ptr ActorFactory::createCheckPointActor(Vector3 p_Position, Vector3 p_Scale, float p_StartTime)
{
	Vector3 AABBScale = p_Scale;
	AABBScale.x *= 1.66f;
	AABBScale.y *= 2.f;
	AABBScale.z *= 1.66f;

	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);

	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", "Checkpoint1");
	pushVector(printer, "Scale", Vector3(0.8f, 0.8f, 0.8f));
	pushVector(printer, "OffsetPosition", Vector3(0,200,0));
	printer.CloseElement();

	printer.OpenElement("ModelSinOffset");
	printer.PushAttribute("StartTime", p_StartTime);
	pushVector(printer, "Offset", Vector3(0, 50, 0));
	printer.CloseElement();

	printer.OpenElement("Movement");
	pushVector(printer, "RotationalVelocity",Vector3(1.57f,0.f,0.f));
	printer.CloseElement();

	printer.OpenElement("AABBPhysics");
	printer.PushAttribute("CollisionResponse", false);
	pushVector(printer, "Halfsize", AABBScale);
	pushVector(printer, "OffsetPosition", Vector3(0.0f, AABBScale.y, 0.0f));
	printer.CloseElement();

	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", "checkpointSwirl");
	printer.CloseElement();

	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

std::string ActorFactory::getPlayerActorDescription(Vector3 p_Position, std::string p_Username, std::string p_CharacterName, std::string p_CharacterStyle) const
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", p_CharacterName.c_str());
	printer.PushAttribute("Style", p_CharacterStyle.c_str());
	printer.CloseElement();

	printer.OpenElement("PlayerPhysics");
	printer.PushAttribute("RadiusMain", 30.f);
	printer.PushAttribute("RadiusAnkle", 10.f);
	printer.PushAttribute("RadiusHead", 25.f);
	printer.PushAttribute("Mass", 68.f);
	printer.PushAttribute("FallTolerance", 0.5f);
	pushVector(printer, "HalfsizeBox", Vector3(25.f, 60.f, 25.f));
	pushVector(printer, "OffsetPositionSphereMain", Vector3(0.f, 35.f, 0.f));
	pushVector(printer, "OffsetPositionSphereHead", Vector3(0.f, 140.f, 0.f));
	pushVector(printer, "OffsetPositionBox", Vector3(0.f, 110.f, 0.f));
	printer.CloseElement();

	printer.OpenElement("TextComponent");
	printer.PushAttribute("Text", p_Username.c_str());
	printer.OpenElement("TextSettings");
	printer.PushAttribute("Font", "Segoe UI");
	printer.PushAttribute("FontSize", 20);
	printer.PushAttribute("Scale", 1);
	printer.PushAttribute("Rotation", 0);
	printer.CloseElement();
	pushColor(printer, "BackgroundColor", Vector4(0.f, 0.f, 0.f, 0.4f));
	pushColor(printer, "FontColor", Vector4(0.8f, 0.8f, 0.8f, 1.f));
	pushVector(printer, "OffsetPosition", Vector3(0.f, 190.f, 0.f));
	printer.CloseElement();

	printer.OpenElement("Pulse");
	printer.PushAttribute("Length", 0.5f);
	printer.PushAttribute("Strength", 0.5f);
	printer.CloseElement();
	printer.OpenElement("Look");
	pushVector(printer, "OffsetPosition", Vector3(0.f, -10.f, 7.f));
	printer.CloseElement();
	printer.OpenElement("HumanAnimation");
	printer.PushAttribute("Animation", p_CharacterName.c_str());
	printer.CloseElement();
	printer.OpenElement("RunControl");
	printer.PushAttribute("MaxSpeed", 1500.f);
	printer.PushAttribute("MaxSpeedDefault", 900.f);
	printer.PushAttribute("Acceleration", 600.f);
	printer.CloseElement();
	printer.CloseElement();

	return printer.CStr();
}

Actor::ptr ActorFactory::createPlayerActor(Vector3 p_Position, std::string p_Username, std::string p_CharacterName, std::string p_CharacterStyle)
{
	tinyxml2::XMLDocument doc;
	doc.Parse(getPlayerActorDescription(p_Position, p_Username, p_CharacterName, p_CharacterStyle).c_str());

	return createActor(doc.FirstChildElement("Object"));
}

Actor::ptr ActorFactory::createDirectionalLight(Vector3 p_Direction, Vector3 p_Color, float p_Intensity)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Directional");
	pushVector(printer, "Direction", p_Direction);
	printer.OpenElement("Intensity");
	printer.PushAttribute("Intensity", p_Intensity);
	printer.CloseElement();
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createSpotLight(Vector3 p_Position, Vector3 p_Direction, Vector2 p_MinMaxAngles, float p_Range, Vector3 p_Color)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Spot");
	printer.PushAttribute("Range", p_Range);
	pushVector(printer, "Position", p_Position);
	pushVector(printer, "Direction", p_Direction);
	printer.OpenElement("Angles");
	printer.PushAttribute("min", p_MinMaxAngles.x);
	printer.PushAttribute("max", p_MinMaxAngles.y);
	printer.CloseElement();
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createPointLight(Vector3 p_Position, float p_Range, Vector3 p_Color)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Point");
	printer.PushAttribute("Range", p_Range);
	pushVector(printer, "Position", p_Position);
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createParticles( Vector3 p_Position, const std::string& p_Effect )
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", p_Effect.c_str());
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	return createActor(doc.FirstChildElement("Object"));
}

Actor::ptr ActorFactory::createParticles( Vector3 p_Position, const std::string& p_Effect, Vector4 p_BaseColor )
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", p_Effect.c_str());
	pushColor(printer, "BaseColor", p_BaseColor);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	return createActor(doc.FirstChildElement("Object"));
}

Actor::ptr ActorFactory::createFlyingCamera(Vector3 p_Position)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("SpherePhysics");
	printer.PushAttribute("Immovable", false);
	printer.PushAttribute("Radius", 50.f);
	printer.PushAttribute("Mass", 70.f);
	printer.PushAttribute("CollisionResponse", true);
	printer.CloseElement();
	printer.OpenElement("FlyingControl");
	printer.PushAttribute("MaxSpeed", 1000.f);
	printer.PushAttribute("Acceleration", 600.f);
	printer.CloseElement();
	printer.OpenElement("Look");
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr(), printer.CStrSize());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createSplineCamera(Vector3 p_Position)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("SpherePhysics");
	printer.PushAttribute("Immovable", false);
	printer.PushAttribute("Radius", 50.f);
	printer.PushAttribute("Mass", 70.f);
	printer.PushAttribute("CollisionResponse", true);
	printer.CloseElement();
	printer.OpenElement("SplineControl");
	printer.PushAttribute("MaxSpeed", 1000.f);
	printer.PushAttribute("Acceleration", 600.f);
	printer.CloseElement();
	printer.OpenElement("Look");
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr(), printer.CStrSize());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createInstanceActor(
		const InstanceModel& p_Model,
		const std::vector<InstanceBoundingVolume>& p_BoundingVolumes,
		const std::vector<InstanceEdgeBox>& p_Edges)
{
	std::string desc = getInstanceActorDescription(p_Model, p_BoundingVolumes, p_Edges);

	tinyxml2::XMLDocument doc;
	doc.Parse(desc.c_str());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

std::string ActorFactory::getInstanceActorDescription(
		const InstanceModel& p_Model,
		const std::vector<InstanceBoundingVolume>& p_BoundingVolumes,
		const std::vector<InstanceEdgeBox>& p_Edges)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Model.position);
	pushRotation(printer, p_Model.rotation);
	print(printer, p_Model);
	for (const auto& volume : p_BoundingVolumes)
	{
		print(printer, volume);
	}
	for (const auto& edge : p_Edges)
	{
		print(printer, edge, p_Model.scale);
	}
	printer.CloseElement();

	return std::string(printer.CStr(), printer.CStrSize() - 1);
}

Actor::ptr ActorFactory::createSpell(const std::string& p_Spell, Actor::Id p_CasterId, Vector3 p_Direction, Vector3 p_StartPosition)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_StartPosition);

	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", "ExplosionSphere1");
	pushVector(printer, "Scale", Vector3(0.02f, 0.02f, 0.02f));
	printer.CloseElement();

	printer.OpenElement("Spell");
	printer.PushAttribute("SpellName", p_Spell.c_str());
	printer.PushAttribute("CasterId", p_CasterId);
	pushVector(printer, "Direction", p_Direction);
	printer.CloseElement();
	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", "magic");
	printer.CloseElement();
	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", "magicProjectile");
	printer.CloseElement();
	//printer.OpenElement("Sound");
	//printer.PushAttribute("FileName", "InAir");
	//printer.PushAttribute("SoundID", 10);
	//printer.PushAttribute("MultiD", 1);
	//printer.PushAttribute("Loop", 1);
	//printer.PushAttribute("MinDistance", 50.0f);
	//pushVector(printer, "Velocity", Vector3(0,0,0));
	//printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

ActorComponent::ptr ActorFactory::createComponent(const tinyxml2::XMLElement* p_Data)
{
	std::string name(p_Data->Value());

	ActorComponent::ptr component;

	auto findIt = m_ComponentCreators.find(name);
	if (findIt != m_ComponentCreators.end())
	{
		componentCreatorFunc creator = findIt->second;
		component = creator();
	}
	else
	{
		throw CommonException("Could not find ActorComponent creator named '" + name + "'", __LINE__, __FILE__);
	}

	if (component)
	{
		component->initialize(p_Data);
	}

	return component;
}

unsigned int ActorFactory::getNextActorId()
{
	return ++m_LastActorId;
}

ActorComponent::ptr ActorFactory::createPlayerComponent()
{
	PlayerBodyComponent* comp = new PlayerBodyComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createOBBComponent()
{
	OBB_Component* comp = new OBB_Component;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createCollisionSphereComponent()
{
	CollisionSphereComponent* comp = new CollisionSphereComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createAABBComponent()
{
	AABB_Component* comp = new AABB_Component;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createBoundingMeshComponent()
{
	BoundingMeshComponent* comp = new BoundingMeshComponent;
	comp->setPhysics(m_Physics);
	comp->setResourceManager(m_ResourceManager);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createModelComponent()
{
	ModelComponent* comp = new ModelComponent;
	comp->setId(++m_LastModelComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createModelSinOffsetComponent()
{
	return ActorComponent::ptr(new ModelSinOffsetComponent);
}

ActorComponent::ptr ActorFactory::createMovementComponent()
{
	return ActorComponent::ptr(new MovementComponent);
}

ActorComponent::ptr ActorFactory::createCircleMovementComponent()
{
	return ActorComponent::ptr(new CircleMovementComponent);
}

ActorComponent::ptr ActorFactory::createSoundComponent()
{
	return ActorComponent::ptr(new SoundComponent);
}

ActorComponent::ptr ActorFactory::createPulseComponent()
{
	return ActorComponent::ptr(new PulseComponent);
}

ActorComponent::ptr ActorFactory::createLightComponent()
{
	LightComponent* comp = new LightComponent;
	comp->setId(++m_LastLightComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createParticleComponent()
{
	ParticleComponent* comp = new ParticleComponent;
	comp->setId(++m_LastParticleComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createSpellComponent()
{
	SpellComponent* comp = new SpellComponent;
	comp->setResourceManager(m_ResourceManager);
	comp->setSpellFactory(m_SpellFactory);
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createLookComponent()
{
	return ActorComponent::ptr(new LookComponent);
}

ActorComponent::ptr ActorFactory::createHumanAnimationComponent()
{
	HumanAnimationComponent* comp = new HumanAnimationComponent;
	comp->setResourceManager(m_ResourceManager);
	comp->setAnimationLoader(m_AnimationLoader);
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createFlyingControlComponent()
{
	FlyingControlComponent* comp = new FlyingControlComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createSplineControlComponent()
{
	SplineControlComponent* comp = new SplineControlComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createRunControlComponent()
{
	RunControlComponent* comp = new RunControlComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createTextComponent()
{
	TextComponent* comp = new TextComponent;
	comp->setId(++m_LastTextComponentId);
	return ActorComponent::ptr(comp);
}

void ActorFactory::print(tinyxml2::XMLPrinter& p_Printer, const InstanceModel& p_Model)
{
	p_Printer.OpenElement("Model");
	p_Printer.PushAttribute("Mesh", p_Model.meshName.c_str());
	pushVector(p_Printer, "Scale", p_Model.scale);
	p_Printer.CloseElement();
}

void ActorFactory::print(tinyxml2::XMLPrinter& p_Printer, const InstanceBoundingVolume& p_Volume)
{
	p_Printer.OpenElement("MeshPhysics");
	p_Printer.PushAttribute("Mesh", p_Volume.meshName.c_str());
	pushVector(p_Printer, "Scale", p_Volume.scale);
	p_Printer.CloseElement();
}

void ActorFactory::print(tinyxml2::XMLPrinter& p_Printer, const InstanceEdgeBox& p_Edge, Vector3 p_Scale)
{
	p_Printer.OpenElement("OBBPhysics");
	p_Printer.PushAttribute("Immovable", true);
	p_Printer.PushAttribute("Mass", 0.f);
	p_Printer.PushAttribute("IsEdge", true);

	using namespace DirectX;

	XMFLOAT3 position = p_Edge.offsetPosition;
	XMFLOAT3 rotation = p_Edge.offsetRotation;
	XMFLOAT3 halfSize = p_Edge.halfsize;

	if(p_Scale.x == p_Scale.y && p_Scale.x == p_Scale.z)
	{
		halfSize = p_Edge.halfsize * p_Scale.x;
		position = p_Edge.offsetPosition * p_Scale.x;
	}
	else
	{
		XMMATRIX rotMat , scalMat;
		rotMat = XMMatrixRotationRollPitchYaw(rotation.y, rotation.x, rotation.z);
		scalMat = XMMatrixScalingFromVector(XMLoadFloat3(&p_Scale));

		float offsetValue = halfSize.x;
		int index = 0;
		float sideValue = halfSize.y;
		if(halfSize.x < halfSize.y)
		{
			offsetValue = halfSize.y;
			sideValue = halfSize.x;
			index = 1;
		}
		if(offsetValue < halfSize.z)
		{
			offsetValue = halfSize.z;
			index = 2;
		}

		XMVECTOR sizeVector = XMVectorZero();
		sizeVector.m128_f32[index] = offsetValue;

		sizeVector = XMVector3Transform(sizeVector, rotMat);

		XMVECTOR pos1, pos2;
		XMVECTOR centerPos = XMLoadFloat3(&position);
		centerPos.m128_f32[3] = 1.0f;
		pos1 = centerPos + sizeVector;
		pos2 = centerPos - sizeVector;

		pos1 = XMVector3Transform(pos1, scalMat);
		pos2 = XMVector3Transform(pos2, scalMat);

		centerPos = (pos1 + pos2) * 0.5f;

		XMStoreFloat3(&position, centerPos);

		XMVECTOR dirVector = pos1 - centerPos;

		float length = XMVector3Length(dirVector).m128_f32[0];

		halfSize.x = length;
		halfSize.y = sideValue;
		halfSize.z = sideValue;

		XMFLOAT3 direction; 
		XMStoreFloat3(&direction,dirVector);
		rotation.x = -atan2f(direction.z, direction.x);
		rotation.y = 0;
		rotation.z = asinf(direction.y / length);
	}

	pushVector(p_Printer, "Halfsize", halfSize);
	pushVector(p_Printer, "OffsetPosition", position);
	pushRotation(p_Printer, "OffsetRotation", rotation);
	p_Printer.CloseElement();
}
