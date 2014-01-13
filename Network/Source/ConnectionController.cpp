#include "ConnectionController.h"

#include "NetworkLogger.h"

ConnectionController::ConnectionController(Connection::ptr p_Connection, const std::vector<PackageBase::ptr>& p_Prototypes)
	:	m_PackagePrototypes(p_Prototypes),
		m_Connection(std::move(p_Connection))
{
	NetworkLogger::log(NetworkLogger::Level::DEBUG_L, "Creating a connection controller");

	m_Connection->setSaveData(std::bind(&ConnectionController::savePackageCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

bool ConnectionController::isConnected() const
{
	return m_Connection->isConnected();
}

bool ConnectionController::hasError() const
{
	return m_Connection->hasError();
}

void ConnectionController::startListening()
{
	m_Connection->startReading();
}

unsigned int ConnectionController::getNumPackages()
{
	return m_ReceivedPackages.size();
}

Package ConnectionController::getPackage(unsigned int p_Index)
{
	return p_Index;
}

void ConnectionController::clearPackages(unsigned int p_NumPackages)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	m_ReceivedPackages.erase(m_ReceivedPackages.begin(), m_ReceivedPackages.begin() + p_NumPackages);
}

PackageType ConnectionController::getPackageType(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	if (m_ReceivedPackages.size() > p_Package)
		return m_ReceivedPackages[p_Package]->getType();
	else
		return PackageType::RESERVED;
}

void ConnectionController::sendCreateObjects(const char** p_Descriptions, unsigned int p_NumDescriptions, const ObjectInstance* p_Instances, unsigned int p_NumInstances)
{
	CreateObjects package;
	for (unsigned int i = 0; i < p_NumDescriptions; ++i)
	{
		package.m_Descriptions.push_back(std::string(p_Descriptions[i]));
	}
	package.m_Instances.assign(p_Instances, p_Instances + p_NumInstances);

	writeData(package.getData(), (uint16_t)package.getType());
}

unsigned int ConnectionController::getNumCreateObjectDescriptions(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	CreateObjects* createObjects = static_cast<CreateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Descriptions.size();
}

const char* ConnectionController::getCreateObjectDescription(Package p_Package, unsigned int p_Description)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	CreateObjects* createObjects = static_cast<CreateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Descriptions[p_Description].c_str();
}

unsigned int ConnectionController::getNumCreateObjectInstances(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	CreateObjects* createObjects = static_cast<CreateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Instances.size();
}

const ObjectInstance* ConnectionController::getCreateObjectInstances(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	CreateObjects* createObjects = static_cast<CreateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Instances.data();
}

void ConnectionController::sendUpdateObjects(const UpdateObjectData* p_ObjectData, unsigned int p_NumObjects, const char** p_ExtraData, unsigned int p_NumExtraData)
{
	UpdateObjects package;
	for (unsigned int i = 0; i < p_NumExtraData; ++i)
	{
		package.m_Extra.push_back(std::string(p_ExtraData[i]));
	}
	package.m_ObjectUpdates.assign(p_ObjectData, p_ObjectData + p_NumObjects);

	writeData(package.getData(), (uint16_t)package.getType());
}

unsigned int ConnectionController::getNumUpdateObjectData(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	UpdateObjects* createObjects = static_cast<UpdateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_ObjectUpdates.size();
}

const UpdateObjectData* ConnectionController::getUpdateObjectData(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	UpdateObjects* createObjects = static_cast<UpdateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_ObjectUpdates.data();
}

unsigned int ConnectionController::getNumUpdateObjectExtraData(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	UpdateObjects* createObjects = static_cast<UpdateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Extra.size();
}

const char* ConnectionController::getUpdateObjectExtraData(Package p_Package, unsigned int p_ExtraData)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	UpdateObjects* createObjects = static_cast<UpdateObjects*>(m_ReceivedPackages[p_Package].get());
	return createObjects->m_Extra[p_ExtraData].c_str();
}

void ConnectionController::sendRemoveObjects(const uint16_t* p_Objects, unsigned int p_NumObjects)
{
	RemoveObjects package;
	package.m_Objects.assign(p_Objects, p_Objects + p_NumObjects);

	writeData(package.getData(), (uint16_t)package.getType());
}

unsigned int ConnectionController::getNumRemoveObjectRefs(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	RemoveObjects* removeObjects = static_cast<RemoveObjects*>(m_ReceivedPackages[p_Package].get());
	return removeObjects->m_Objects.size();
}

const uint16_t* ConnectionController::getRemoveObjectRefs(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	RemoveObjects* removeObjects = static_cast<RemoveObjects*>(m_ReceivedPackages[p_Package].get());
	return removeObjects->m_Objects.data();
}

void ConnectionController::sendObjectAction(uint16_t p_ObjectId, const char* p_Action)
{
	ObjectAction package;
	package.m_Object = p_ObjectId;
	package.m_Action = p_Action;

	writeData(package.getData(), (uint16_t)package.getType());
}

uint16_t ConnectionController::getObjectActionId(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	ObjectAction* objectAction = static_cast<ObjectAction*>(m_ReceivedPackages[p_Package].get());
	return objectAction->m_Object;
}

const char* ConnectionController::getObjectActionAction(Package p_Package)
{
	std::lock_guard<std::mutex> lock(m_ReceivedLock);
	ObjectAction* objectAction = static_cast<ObjectAction*>(m_ReceivedPackages[p_Package].get());
	return objectAction->m_Action.c_str();
}

void ConnectionController::setDisconnectedCallback(Connection::disconnectedCallback_t p_DisconnectCallback)
{
	m_Connection->setDisconnectedCallback(p_DisconnectCallback);
}

void ConnectionController::writeData(const std::string& p_Buffer, uint16_t p_ID)
{
	if (m_Connection)
	{
		m_Connection->writeData(p_Buffer, p_ID);
	}
}

void ConnectionController::savePackageCallBack(uint16_t p_ID, const std::string& p_Data)
{
	for(const std::unique_ptr<PackageBase>& p : m_PackagePrototypes)
	{
		if(p->getType() == (PackageType)p_ID)
		{
			PackageBase::ptr package = p->createPackage(p_Data);
			std::lock_guard<std::mutex> lock(m_ReceivedLock);
			m_ReceivedPackages.push_back(std::move(package));
			return;
		}
 	}

	std::string msg("Received unregistered package type: " + std::to_string(p_ID));
	NetworkLogger::log(NetworkLogger::Level::WARNING, msg);
}
