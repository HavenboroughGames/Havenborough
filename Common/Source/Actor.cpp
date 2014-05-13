#include "Actor.h"
#include "ActorList.h"
#include "Components.h"

Actor::Actor(Id p_Id, EventManager* p_EventManager, std::weak_ptr<ActorList> p_ActorList)
	: m_Id(p_Id), m_EventManager(p_EventManager),
		m_ActorList(p_ActorList)
{
}

Actor::~Actor()
{
}

void Actor::initialize(const tinyxml2::XMLElement* p_Data)
{
	m_Position = Vector3(0.f, 0.f, 0.f);
	m_Rotation = Vector3(0.f, 0.f, 0.f);

	p_Data->QueryAttribute("x", &m_Position.x);
	p_Data->QueryAttribute("y", &m_Position.y);
	p_Data->QueryAttribute("z", &m_Position.z);

	p_Data->QueryAttribute("yaw", &m_Rotation.x);
	p_Data->QueryAttribute("pitch", &m_Rotation.y);
	p_Data->QueryAttribute("roll", &m_Rotation.z);
}

void Actor::postInit()
{
	for (auto& comp : m_Components)
	{
		comp->postInit();
	}
}

void Actor::onUpdate(float p_DeltaTime)
{
	for (auto& comp : m_Components)
	{
		comp->onUpdate(p_DeltaTime);
	}
}

Actor::Id Actor::getId() const
{
	return m_Id;
}

Vector3 Actor::getPosition() const
{
	return m_Position;
}

void Actor::setPosition(Vector3 p_Position)
{
	for (auto& comp : m_Components)
	{
		comp->setPosition(p_Position);
	}

	m_Position = p_Position;
}

Vector3 Actor::getRotation() const
{
	return m_Rotation;
}

void Actor::setRotation(Vector3 p_Rotation)
{
	for (auto& comp : m_Components)
	{
		comp->setRotation(p_Rotation);
	}

	m_Rotation = p_Rotation;
}

EventManager* Actor::getEventManager() const
{
	return m_EventManager;
}

std::vector<BodyHandle> Actor::getBodyHandles() const
{
	std::vector<BodyHandle> bodies;
	
	for(auto &comp : m_Components)
	{
		if(comp->getComponentId() == PhysicsInterface::m_ComponentId)
		{
			bodies.push_back(std::static_pointer_cast<PhysicsInterface>(comp)->getBodyHandle());
		}
		else if (comp->getComponentId() == SpellInterface::m_ComponentId)
		{
			BodyHandle handle = std::static_pointer_cast<SpellInterface>(comp)->getBodyHandle();
			if (handle != 0)
			{
				bodies.push_back(handle);
			}
		}
	}
	
	return bodies;
}

void Actor::serialize(std::ostream& p_Stream) const
{
	tinyxml2::XMLPrinter printer;
	serialize(printer);

	p_Stream << printer.CStr();
}

void Actor::serialize(tinyxml2::XMLPrinter& p_Printer) const
{
	p_Printer.OpenElement("Object");

	p_Printer.PushAttribute("x", m_Position.x);
	p_Printer.PushAttribute("y", m_Position.y);
	p_Printer.PushAttribute("z", m_Position.z);
	p_Printer.PushAttribute("yaw", m_Rotation.x);
	p_Printer.PushAttribute("pitch", m_Rotation.y);
	p_Printer.PushAttribute("roll", m_Rotation.z);

	for (const auto& comp : m_Components)
	{
		comp->serialize(p_Printer);
	}

	p_Printer.CloseElement();
}

void Actor::addComponent(ActorComponent::ptr p_Component)
{
	m_Components.push_back(p_Component);
}

DirectX::XMFLOAT4X4 Actor::getWorldMatrix() const
{
	using namespace DirectX;

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Rotation.y, m_Rotation.x, m_Rotation.z);
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&XMFLOAT3(m_Position)));

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranspose(rotation * translation));

	return world;
}

Actor::ptr Actor::findActor(Actor::Id p_ActorId) const
{
	std::shared_ptr<ActorList> list = m_ActorList.lock();
	if (list)
	{
		return list->findActor(p_ActorId);
	}
	else
	{
		return Actor::ptr();
	}
}
