#include "SceneManager.h"
#include "MyExceptions.h"

SceneManager::SceneManager()
{
	//m_MenuSceneList.clear();
	//m_RunSceneList.clear();
	m_NowShowing = 0;
}

SceneManager::~SceneManager()
{

}

bool SceneManager::init()
{
	m_MenuSceneList.resize(2);
	m_RunSceneList.resize(3);

	m_MenuSceneList[0] = IScene::ptr(new MenuScene);
	m_MenuSceneList[1] = IScene::ptr(new MenuScene);

	m_RunSceneList[0] = IScene::ptr(new GameScene);
	m_RunSceneList[1] = IScene::ptr(new GameScene);
	m_RunSceneList[2] = IScene::ptr(new MenuScene);

	m_NumberOfMenuScene = m_MenuSceneList.size();
	m_NumberOfRunScene = m_RunSceneList.size();

	bool sceneFail = false;
	unsigned int i;
	for(i = 0; i < m_NumberOfMenuScene; i++)
	{
		if(!m_MenuSceneList[i]->init(i))
		{
			sceneFail = true;
		}
	}
	for(i = 0; i < m_NumberOfRunScene; i++)
	{
		if(!m_RunSceneList[i]->init(i))
		{
			sceneFail = true;
		}
	}
	m_MenuSceneList[0]->setIsVisible(true);

	if(sceneFail)
	{
		throw SceneManagerException("Failed to init all scenes", __LINE__,__FILE__);
		return false;
	}

	return true;
}

void SceneManager::destroy()
{
	unsigned int i;
	for(i = 0; i < m_NumberOfMenuScene; i++)
	{
		m_MenuSceneList[i]->destroy();
	}
	for(i = 0; i < m_NumberOfRunScene; i++)
	{
		m_RunSceneList[i]->destroy();
	}
	m_MenuSceneList.clear();
	m_RunSceneList.clear();
}

void SceneManager::onFrame()
{
	std::vector<IScene::ptr>* activeList = nullptr;
	unsigned int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(unsigned int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->onFrame(&m_NowShowing);
			if(i != m_NowShowing)
			{
				i = nrScenes;
			}
		}
	}

	if(m_NowShowing != -1)
	{
		changeScene(m_NowShowing);
	}
	else
	{
		if(m_IsMenuState)
		{
			startRun();
		}
		else
		{
			startMenu();
		}
	}
	activeList = nullptr;
}

void SceneManager::render()
{
	std::vector<IScene::ptr>* activeList = nullptr;
	unsigned int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(unsigned int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->render();
		}
	}
	activeList = nullptr;
}

void SceneManager::setPause()
{
	if(!m_IsMenuState)
	{
		bool currentState;
		currentState = m_RunSceneList[GAMEPAUSE]->getIsVisible();
		m_RunSceneList[GAMEPAUSE]->setIsVisible(!currentState);
	}
}

void SceneManager::changeScene(int p_NowShowing)
{
	if(m_IsMenuState)
	{
		m_MenuSceneList[p_NowShowing]->setIsVisible(true);
		m_MenuSceneList[p_NowShowing]->onFrame(&m_NowShowing);
	}
	else
	{
		m_RunSceneList[p_NowShowing]->setIsVisible(true);
	}
}

void SceneManager::startRun()
{
	m_IsMenuState = false;
	m_RunSceneList[0]->setIsVisible(true);
	m_RunSceneList[1]->setIsVisible(true);
	for(unsigned int i = 2; i < m_NumberOfRunScene; i++)
	{
		m_RunSceneList[i]->setIsVisible(false);
	}
	m_NowShowing = 0;
}

void SceneManager::startMenu()
{
	m_IsMenuState = true;
	m_MenuSceneList[0]->setIsVisible(true);
	for(unsigned int i = 1; i < m_NumberOfMenuScene; i++)
	{
		m_MenuSceneList[i]->setIsVisible(false);
	}
	m_NowShowing = 0;
}

bool SceneManager::keyStroke(WPARAM p_WParam, LPARAM p_LParam, LRESULT& p_Result)
{
	if(p_WParam == 'K')
	{
		setPause();
		return true;
	}
	//Change scene
	else if(p_WParam == 'L' || p_WParam == 'J')
	{
		passKeyStroke((char)p_WParam);
		return true;
	}
	return false;
}

void SceneManager::passKeyStroke(char p_key)
{
	std::vector<IScene::ptr>* activeList = nullptr;
	unsigned int nrScenes = 0;

	if(m_IsMenuState)
	{
		activeList = &m_MenuSceneList;
		nrScenes = m_NumberOfMenuScene;
	}
	else
	{
		activeList = &m_RunSceneList;
		nrScenes = m_NumberOfRunScene;
	}

	for(unsigned int i = 0; i < nrScenes; i++)
	{
		if(activeList->at(i)->getIsVisible())
		{
			activeList->at(i)->registeredKeyStroke(&p_key);
			i = nrScenes;
		}
	}
	activeList = nullptr;
}

/*########## TEST FUNCTIONS ##########*/

std::vector<IScene::ptr> SceneManager::getScene()
{
	std::vector<IScene::ptr> temp;
	if(m_IsMenuState)
	{
		for(unsigned int i = 0; i < m_NumberOfMenuScene; i++)
		{
			if(m_MenuSceneList[i]->getIsVisible() == true)
			{
				temp.push_back(m_MenuSceneList[i]);
			}
		}
	}
	else
	{
		for(unsigned int i = 0; i < m_NumberOfRunScene; i++)
		{
			if(m_RunSceneList[i]->getIsVisible() == true)
			{
				temp.push_back(m_RunSceneList[i]);
			}
		}
	}
	return temp;
}
