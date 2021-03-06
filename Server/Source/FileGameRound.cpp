#include "FileGameRound.h"

#include <Components.h>
#include <Logger.h>
#include <LookComponent.h>
#include <XMLHelper.h>

#include <fstream>
#include <sstream>

using namespace DirectX;

static const float spawnEpsilon = 100.f;

void FileGameRound::setup()
{
	m_FileLoader.reset(new InstanceBinaryLoader);
	m_FileLoader->loadBinaryFile(m_FilePath);
	m_Random.seed((unsigned long)std::chrono::system_clock::now().time_since_epoch().count());

	createPlayerActors();
	createCheckpoints();

	m_PlayerPositionList = m_Players;

	m_Time = 0.f;
	m_ResultListUpdated = false;
}

void FileGameRound::setFilePath(std::string p_Filepath)
{
	m_FilePath = p_Filepath;
}

void FileGameRound::sendLevel()
{
	std::vector<std::string> descriptions;
	std::vector<ObjectInstance> instances;

	for (const auto& actor : m_Actors)
	{
		std::ostringstream descStream;
		actor->serialize(descStream);
		descriptions.push_back(descStream.str());
		ObjectInstance inst = 
		{
			descriptions.back().c_str(),
			actor->getId()
		};
		instances.push_back(inst);
	}

	std::string stream = m_FileLoader->getDataStream();
	m_FileLoader.reset();

	for (auto& player : m_Players)
	{
		User::ptr user = player->getUser().lock();
		if (user)
		{
			Actor::ptr actor = player->getActor().lock();
			if(actor)
			{
				user->getConnection()->sendCreateObjects(instances.data(), instances.size());
				user->getConnection()->sendCurrentCheckpoint(player->getCurrentCheckpoint()->getPosition() + Vector3(0.f, spawnEpsilon, 0.f));
				user->getConnection()->sendLevelData(stream.c_str(), stream.size());
				user->getConnection()->sendNrOfCheckpoints(player->getNumberOfCheckpoints());
				user->getConnection()->sendAssignPlayer(actor->getId());
			}
		}
	}
}

void FileGameRound::updateLogic(float p_DeltaTime)
{
	m_Time += p_DeltaTime;
	for (auto& actor : m_Actors)
	{
		actor->onUpdate(p_DeltaTime);
	}
	for(int i = m_Physics->getHitDataSize()-1 ; i >= 0; i--)
	{
		HitData hit = m_Physics->getHitDataAt(i);
		if (m_Players.size() != 0)
		{
			Player::ptr player = findPlayer(hit.collider);
			Actor::ptr victim = findActor(hit.collisionVictim);

			if (player && victim)
			{
				if(!player->reachedFinishLine())
				{
					if(player->getCurrentCheckpointBodyHandle() == hit.collisionVictim)
					{
						m_SendHitData.push_back(std::make_pair(player, victim));
						player->changeCheckpoint();
						player->clockPosition(m_Time);
						rearrangePlayerPosition();
					}
				}
			}
		}
	}
}

void FileGameRound::handleExtraPackage(Player::ptr p_Player, Package p_Package)
{
	User::ptr user = p_Player->getUser().lock();
	if (!user)
	{
		return;
	}

	IConnectionController* con = user->getConnection();
	PackageType type = con->getPackageType(p_Package);

	switch (type)
	{
	case PackageType::THROW_SPELL:
		handleThrowSpell(p_Player, p_Package, con);
		break;

	case PackageType::OBJECT_ACTION:
		handleObjectAction(p_Player, p_Package, con);
		break;

	default:
		GameRound::handleExtraPackage(p_Player, p_Package);
		break;
	}
}

void FileGameRound::sendUpdates()
{
	std::vector<UpdateObjectData> data;
	std::vector<std::string> extra;
	std::vector<const char*> extraC;
	for (auto& player : m_Players)
	{
		data.push_back(getUpdateData(player));
		extra.push_back(getExtraData(player));
		extraC.push_back(extra.back().c_str());
	}
	for (auto& player : m_Players)
	{
		User::ptr user = player->getUser().lock();
		if (user)
		{
			user->getConnection()->sendUpdateObjects(data.data(), data.size(), extraC.data(), extraC.size());
		}
	}

	const bool updatePositions = !m_SendHitData.empty();
	for(const auto& hitData : m_SendHitData)
	{
		Player::ptr player = hitData.first;
		User::ptr user = player->getUser().lock();
		if (user)
		{
			unsigned int checkpointIndex = player->getNrOfCheckpointsTaken();
			Actor::ptr actor = hitData.second.lock();
			if (actor)
			{
				const Actor::Id id = actor->getId();
				
				user->getConnection()->sendRemoveObjects(&id, 1);
				user->getConnection()->sendTakenCheckpoints(player->getNrOfCheckpointsTaken());
				user->getConnection()->sendSetSpawnPosition(actor->getPosition() + Vector3(0.f, spawnEpsilon, 0.f));

				const float leadTime = m_PlayerPositionList[0]->getClockedTimeAtCheckpoint(checkpointIndex);
				const float playerTime = player->getClockedTimeAtCheckpoint(checkpointIndex);

				if(!player->reachedFinishLine())
				{
					sendSelectNextCheckpoint(player, user);
					const float timeDiff = playerTime - leadTime;
					sendPositionUpdate(player, &timeDiff);
				}
				else
				{
					m_ResultList.push_back(std::make_pair(user->getUsername(), playerTime));
					m_ResultListUpdated = true;
						
					replacePlayerActorWithFlyingCamera(player, user);
				}
			}
		}
	}
	m_SendHitData.clear();

	if (updatePositions)
	{
		sendPositionUpdates();
	}

	if(m_ResultListUpdated && countPlayersRacing() < m_Players.size())
	{
		sendResultLists();
	}
}

void FileGameRound::playerDisconnected(Player::ptr p_DisconnectedPlayer)
{
	Actor::ptr actor = p_DisconnectedPlayer->getActor().lock();
	if (!actor)
	{
		return;
	}

	Actor::Id playerActorId = actor->getId();

	for (auto& player : m_Players)
	{
		User::ptr user = player->getUser().lock();
		if (user)
		{
			user->getConnection()->sendRemoveObjects(&playerActorId, 1);
		}
	}

	auto playerPosition = std::find(m_PlayerPositionList.begin(), m_PlayerPositionList.end(), p_DisconnectedPlayer);
	if (playerPosition != m_PlayerPositionList.end())
	{
		m_PlayerPositionList.erase(playerPosition);
	}

	auto it = std::find(m_Actors.begin(), m_Actors.end(), actor);
	if (it != m_Actors.end())
	{
		m_Actors.erase(it);
	}
}

UpdateObjectData FileGameRound::getUpdateData(const Player::ptr p_Player)
{
	Actor::ptr actor = p_Player->getActor().lock();
	
	if (!actor)
	{
		throw CommonException("Player missing actor", __LINE__, __FILE__);
	}

	std::shared_ptr<PhysicsInterface> physComp = actor->getComponent<PhysicsInterface>(PhysicsInterface::m_ComponentId).lock();

	Vector3 velocity(0.f, 0.f, 0.f);
	Vector3 rotVelocity(0.f, 0.f, 0.f);

	if (physComp)
	{
		velocity = physComp->getVelocity();
	}

	UpdateObjectData data =
	{
		actor->getPosition(),
		velocity,
		actor->getRotation(),
		rotVelocity,
		actor->getId()
	};

	return data;
}

std::string FileGameRound::getExtraData(const Player::ptr p_Player)
{
	Actor::ptr actor = p_Player->getActor().lock();

	if (!actor)
		return "";

	tinyxml2::XMLPrinter printer;
	printer.OpenElement("ObjectUpdate");
	printer.PushAttribute("ActorId", actor->getId());
	printer.PushAttribute("Type", "Look");
	p_Player->getActor().lock()->getComponent<LookInterface>(LookInterface::m_ComponentId).lock()->serialize(printer);
	printer.CloseElement();

	return printer.CStr();
}

Player::ptr FileGameRound::findPlayer(BodyHandle p_Body)
{
	auto playerIt = std::find_if(m_Players.begin(), m_Players.end(),
	[&p_Body] (Player::ptr p_Player)
	{
		Actor::ptr actor = p_Player->getActor().lock();
		if (!actor)
			return false;

		for (auto& body : actor->getBodyHandles())
		{
			if (body == p_Body)
			{
				return true;
			}
		}

		return false;
	});

	if (playerIt == m_Players.end())
	{
		return Player::ptr();
	}
	else
	{
		return *playerIt;
	}
}

Actor::ptr FileGameRound::findActor(BodyHandle p_Body)
{
	auto actorIt = std::find_if(m_Actors.begin(), m_Actors.end(),
	[&p_Body] (Actor::ptr p_Actor)
	{
		if (!p_Actor)
		{
			return false;
		}

		for (auto& body : p_Actor->getBodyHandles())
		{
			if (body == p_Body)
			{
				return true;
			}
		}
		return false;
	});

	if (actorIt == m_Actors.end())
	{
		return Actor::ptr();
	}
	else
	{
		return *actorIt;
	}
}

void FileGameRound::rearrangePlayerPosition()
{
	std::sort(m_PlayerPositionList.begin(), m_PlayerPositionList.end(),
		[] (const Player::ptr p_Left, const Player::ptr p_Right)
	{
		const unsigned int leftNum = p_Left->getNrOfCheckpointsTaken();
		const unsigned int rightNum = p_Right->getNrOfCheckpointsTaken();

		if (rightNum < leftNum)
			return true;
		if (leftNum < rightNum)
			return false;

		return p_Left->getClockedTimeAtCheckpoint(leftNum) < p_Right->getClockedTimeAtCheckpoint(rightNum);
	});
}

unsigned int FileGameRound::getPlayerPos(Player::ptr p_Player) const
{
	for(unsigned int i = 0; i < m_PlayerPositionList.size(); i++)
	{
		if(m_PlayerPositionList[i] == p_Player)
		{
			return i+1;
		}
	}
	throw std::exception("Error in function getPlayerPos!", __LINE__);
}

unsigned int FileGameRound::countPlayersRacing() const
{
	unsigned int count = 0;
	for (const auto& player : m_Players)
	{
		if (!player->reachedFinishLine())
		{
			++count;
		}
	}
	return count;
}

void FileGameRound::createPlayerActors()
{
	const Vector3 basePos = Vector3(m_FileLoader->getCheckPointStart()) + Vector3(0.f, spawnEpsilon, 0.f);
	const float angle = 2 * PI / m_Players.size();
	for (size_t i = 0; i < m_Players.size(); ++i)
	{
		User::ptr user = m_Players[i]->getUser().lock();
		if (!user)
		{
			continue;
		}

		static const float spawnCircleRadius = 200.f;
		Vector3 position = basePos + Vector3(sinf(i * angle), 0.f, cosf(i * angle)) * spawnCircleRadius;

		Actor::ptr actor = m_ActorFactory->createPlayerActor(
			position, user->getUsername(),
			user->getCharacterName(), user->getCharacterStyle());
		m_Players[i]->setActor(actor);
		m_Actors.push_back(actor);
	}
}

void FileGameRound::createCheckpoints()
{
	std::vector<InstanceBinaryLoader::CheckPointStruct> checkpoints;

	for(const auto& checkpointGroup : m_FileLoader->getCheckPointData())
	{
		if (checkpointGroup.empty())
			continue;

		std::uniform_int_distribution<int> randomCheckpoint(0, checkpointGroup.size() - 1);
		checkpoints.push_back(checkpointGroup[randomCheckpoint(m_Random)]);
	}

	std::sort(checkpoints.begin(), checkpoints.end(),
		[] (const InstanceBinaryLoader::CheckPointStruct& p_Left, const InstanceBinaryLoader::CheckPointStruct& p_Right)
		{
			return p_Left.m_Number > p_Right.m_Number;
		});
	
	static const Vector3 checkpointScale(54.f, 170.f, 54.f);

	std::vector<Actor::ptr> checkpointList;
	std::uniform_real_distribution<float> circleDist(0.f, PI * 2.f);
	checkpointList.push_back(m_ActorFactory->createCheckPointActor(m_FileLoader->getCheckPointEnd(), checkpointScale, circleDist(m_Random)));
	for (const auto& checkpoint : checkpoints)
	{
		checkpointList.push_back(m_ActorFactory->createCheckPointActor(checkpoint.m_Translation, checkpointScale, circleDist(m_Random)));
	}

	for (const auto& checkpoint : checkpointList)
	{
		m_Actors.push_back(checkpoint);
		for(auto& player : m_Players)
		{
			player->addCheckpoint(checkpoint);
		}
	}
}

void FileGameRound::sendPositionUpdate(const Player::ptr p_Player, const float* p_Time) const
{
	User::ptr user = p_Player->getUser().lock();
	if(!user)
	{
		return;
	}

	tinyxml2::XMLPrinter printer;
	printer.OpenElement("RacePositions");
	printer.PushAttribute("Place", getPlayerPos(p_Player));
	if (p_Time)
		printer.PushAttribute("Time", *p_Time);
	printer.CloseElement();
	const char* info = printer.CStr();
	user->getConnection()->sendRacePosition(&info,1);
}

void FileGameRound::sendPositionUpdates() const
{
	for(const auto& player : m_Players)
	{
		sendPositionUpdate(player, nullptr);
	}
}

void FileGameRound::sendResultLists() const
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("GameResult");
	printer.PushAttribute("Type", "Result");
	printer.OpenElement("ResultList");
	for(unsigned int i = 0; i < m_ResultList.size(); i++)
	{
		printer.OpenElement("Place");
			
		const auto& result = m_ResultList[i];
				
		printer.PushAttribute("Player", result.first.c_str());
		printer.PushAttribute("Time", result.second);
		printer.CloseElement();
	}
	printer.CloseElement();
	printer.CloseElement();
	const char* info = printer.CStr();
	for(auto& player : m_Players)
	{
		if (player->reachedFinishLine())
		{
			User::ptr user = player->getUser().lock();
			if (user)
				user->getConnection()->sendGameResult(&info, 1);
		}
	}
}

void FileGameRound::sendSelectNextCheckpoint(const Player::ptr p_Player, const User::ptr p_User) const
{
	tinyxml2::XMLPrinter printer;				

	printer.OpenElement("ObjectUpdate");
	printer.PushAttribute("ActorId", p_Player->getCurrentCheckpoint()->getId());
	printer.PushAttribute("Type", "Color");
	pushColor(printer, "SetColor", p_Player->getCurrentCheckpointColor());
	printer.CloseElement();
	const char* info = printer.CStr();
	p_User->getConnection()->sendUpdateObjects(NULL, 0, &info, 1);

	p_User->getConnection()->sendCurrentCheckpoint(p_Player->getCurrentCheckpoint()->getPosition());
}

void FileGameRound::handleThrowSpell(const Player::ptr p_Player, Package p_Package, IConnectionController* p_Connection)
{
	Actor::ptr playerActor = p_Player->getActor().lock();
	if (!playerActor)
	{
		return;
	}

	Actor::ptr spellActor = m_ActorFactory->createSpell(
		p_Connection->getThrowSpellName(p_Package),
		playerActor->getId(),
		p_Connection->getThrowSpellDirection(p_Package),
		p_Connection->getThrowSpellStartPosition(p_Package));

	std::ostringstream outStream;
	spellActor->serialize(outStream);
	std::string spellDescription = outStream.str();

	ObjectInstance spellInstance;
	spellInstance.m_Id = spellActor->getId();
	spellInstance.m_Description = spellDescription.c_str();

	for (auto& player : m_Players)
	{
		if (player == p_Player)
		{
			continue;
		}

		User::ptr otherUser = player->getUser().lock();
		if (!otherUser)
		{
			continue;
		}

		otherUser->getConnection()->sendCreateObjects(&spellInstance, 1);
	}
}

void FileGameRound::handleObjectAction(const Player::ptr p_Player, Package p_Package, IConnectionController* p_Connection)
{
	Actor::Id actor = p_Connection->getObjectActionId(p_Package);
	const char* action = p_Connection->getObjectActionAction(p_Package);

	for (auto& player : m_Players)
	{
		if (player == p_Player)
		{
			continue;
		}

		User::ptr otherUser = player->getUser().lock();
		if (!otherUser)
		{
			continue;
		}

		otherUser->getConnection()->sendObjectAction(actor, action);
	}
}

void FileGameRound::replacePlayerActorWithFlyingCamera(Player::ptr p_Player, const User::ptr p_User)
{
	Actor::ptr oldPlayerActor = p_Player->getActor().lock();
	Actor::ptr flyingCamera = m_ActorFactory->createFlyingCamera(
		oldPlayerActor->getComponent<LookComponent>(LookComponent::m_ComponentId).lock()->getLookPosition());
	m_Actors.push_back(flyingCamera);

	std::ostringstream oStream;
	flyingCamera->serialize(oStream);
	ObjectInstance inst;
	std::string desc = oStream.str();
	inst.m_Description = desc.c_str();
	inst.m_Id = flyingCamera->getId();

	for (auto& otherPlayer : m_Players)
	{
		User::ptr otherUser = otherPlayer->getUser().lock();
		if(!otherUser)
		{
			continue;
		}
		otherUser->getConnection()->sendCreateObjects(&inst, 1);
	}

	p_User->getConnection()->sendAssignPlayer(inst.m_Id);

	Actor::Id oldPlayerId = oldPlayerActor->getId();

	for (auto& player : m_Players)
	{
		User::ptr otherUser = player->getUser().lock();
		if(!otherUser)
		{
			continue;
		}
		otherUser->getConnection()->sendRemoveObjects(&oldPlayerId, 1);
	}

	p_Player->setActor(flyingCamera);

	auto actorIt = std::find(m_Actors.begin(), m_Actors.end(), oldPlayerActor);
	if (actorIt != m_Actors.end())
	{
		m_Actors.erase(actorIt);
	}
}
