#pragma once 
#include "Scenes/IScene.h"

#include <vector>

class SceneManager
{
private:
	std::vector<IScene::ptr> m_MenuSceneList;
	std::vector<IScene::ptr> m_RunSceneList;
	unsigned int			 m_NumberOfMenuScene;
	unsigned int			 m_NumberOfRunScene;
	int						 m_NowShowing;
	/**
	* Changes render and update lists of scenes.
	* true = Menu(m_MenuSceneList)
	* false = Run(m_RunSceneList)
	*/
	bool			m_IsMenuState;
	IGraphics		*m_Graphics;
	ResourceManager	*m_ResourceManager;
	Input			*m_InputQueue;
	GameLogic		*m_GameLogic;

public:
	SceneManager();
	~SceneManager();

	/**
	* Initialize the scenes and scene manager.
	* @param p_Graphics pointer to the graphics engine
	* @param p_ResourceManager pointer to the resource manager
	* @param p_Physics pointer to the physics engine
	* @param p_InputQueue pointer to the keyboard and mouse input queue
	*/
	void init(IGraphics *p_Graphics, ResourceManager *p_ResourceManager, Input *p_InputQueue,
		GameLogic *p_GameLogic, EventManager *p_EventManager);
	
	/**
	* Destroy scenes when shutdown.
	*/
	void destroy();
	
	/**
	* Update active scenes every frame.
	* @param p_DeltaTime the time between two frames
	*/
	void onFrame(float p_DeltaTime);
	
	/**
	* Render active scenes.
	*/
	void render();

	/**
	* Pass along keyboard and mouse input to a scene.
	* @param p_Action the action to take
	* @param p_Value the value for the action
	* @param p_PrevValue the previous value for the action, when checked if zero there is no key repeats.
	*/
	void registeredInput(std::string p_Action, float p_Value, float p_PrevValue);

	/**
	* Switching to run scene list.
	*/
	void startRun();
	/**
	* Switching to menu scene list.
	*/
	void startMenu();

	IScene::ptr getScene(RunScenes p_Scene);
	
	/*########## TEST FUNCTIONS ##########*/

	std::vector<IScene::ptr> getScene();

	
private:
	/**
	* Change the scene 
	* @param p_NowShowing gives visibility to a new scene.
	*/
	void changeScene(float p_DeltaTime, int p_NowShowing);
	/**
	* Update menu scenes
	*/
	void onFrameMenu();
	/**
	* Update game scenes
	*/
	void onFrameRun();
	/**
	* Set pause screen when game is running.
	*/
	void setPause();

	void passInput(std::string p_Action, float p_Value, float p_PrevValue);
};