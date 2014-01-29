#include "BaseGameApp.h"

#include "ActorFactory.h"
#include "Components.h"
#include "Input\InputTranslator.h"
#include "Logger.h"

#include <iomanip>
#include <sstream>

#include <tinyxml2/tinyxml2.h>

const double pi = 3.14159265358979323846264338;

const std::string BaseGameApp::m_GameTitle = "The Apprentice of Havenborough";

BaseGameApp::BaseGameApp()
	:	m_ActorFactory(0x10000)
{
}

void BaseGameApp::init()
{
	Logger::log(Logger::Level::INFO, "Initializing game app");

	m_GameLogic = nullptr;

	m_MemUpdateDelay = 0.1f;
	m_TimeToNextMemUpdate = 0.f;
	
	m_Window.init(getGameTitle(), getWindowSize());

	m_Graphics = IGraphics::createGraphics();
	m_Graphics->setLogFunction(&Logger::logRaw);
	
	//TODO: Need some input setting variable to handle fullscreen.
	bool fullscreen = false;
	m_Graphics->initialize(m_Window.getHandle(), (int)m_Window.getSize().x, (int)m_Window.getSize().y, fullscreen);
	m_Window.registerCallback(WM_CLOSE, std::bind(&BaseGameApp::handleWindowClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_Window.registerCallback(WM_EXITSIZEMOVE, std::bind(&BaseGameApp::handleWindowExitSizeMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_Window.registerCallback(WM_SIZE, std::bind(&BaseGameApp::handleWindowSize, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_Physics = IPhysics::createPhysics();
	m_Physics->setLogFunction(&Logger::logRaw);
	m_Physics->initialize();

	m_ResourceManager.reset(new ResourceManager());
	
	m_Sound = ISound::createSound();
	m_Sound->setLogFunction(&Logger::logRaw);
	m_Sound->initialize();

	using namespace std::placeholders;
	m_Graphics->setLoadModelTextureCallBack(&ResourceManager::loadModelTexture, m_ResourceManager.get());
	m_Graphics->setReleaseModelTextureCallBack(&ResourceManager::releaseModelTexture, m_ResourceManager.get());
	m_ResourceManager->registerFunction( "model", std::bind(&IGraphics::createModel, m_Graphics, _1, _2), std::bind(&IGraphics::releaseModel, m_Graphics, _1) );
	m_ResourceManager->registerFunction( "texture", std::bind(&IGraphics::createTexture, m_Graphics, _1, _2), std::bind(&IGraphics::releaseTexture, m_Graphics, _1));
	m_ResourceManager->registerFunction( "volume", std::bind(&IPhysics::createBV, m_Physics, _1, _2), std::bind(&IPhysics::releaseBV, m_Physics, _1));
	m_ResourceManager->registerFunction("sound", std::bind(&ISound::loadSound, m_Sound, _1, _2), std::bind(&ISound::releaseSound, m_Sound, _1));

	InputTranslator::ptr translator(new InputTranslator);
	translator->init(&m_Window);
	
	//TODO: This should be loaded from file
	Logger::log(Logger::Level::DEBUG_L, "Adding input mappings");
	translator->addKeyboardMapping(VK_ESCAPE, "back");
	translator->addKeyboardMapping(VK_ESCAPE, "leaveGame");
	translator->addKeyboardMapping('W', "moveForward");
	translator->addKeyboardMapping('S', "moveBackward");
	translator->addKeyboardMapping('A', "moveLeft");
	translator->addKeyboardMapping('D', "moveRight");
	translator->addKeyboardMapping('Z', "changeViewN");
	translator->addKeyboardMapping('X', "changeViewP");
	translator->addKeyboardMapping('I', "toggleIK");
	translator->addKeyboardMapping(VK_SPACE, "jump");
	translator->addKeyboardMapping('C', "connectToServer");
	translator->addKeyboardMapping('T', "joinTestLevel");
	translator->addKeyboardMapping('Y', "joinServerLevel");
	translator->addKeyboardMapping('J', "playLocalTest");

	//translator->addKeyboardMapping('J', "changeSceneP");
	//translator->addKeyboardMapping('K', "pauseScene");
	//translator->addKeyboardMapping('L', "changeSceneN");
	translator->addKeyboardMapping('9', "switchBVDraw");
	translator->addKeyboardMapping(VK_RETURN, "goToMainMenu");

	translator->addKeyboardMapping('B', "blendAnimation");
	translator->addKeyboardMapping('N', "resetAnimation");
	translator->addKeyboardMapping('M', "layerAnimation");
	translator->addKeyboardMapping('V', "resetLayerAnimation");
	
	translator->addMouseMapping(InputTranslator::Axis::HORIZONTAL, "mousePosHori", "mouseMoveHori");
	translator->addMouseMapping(InputTranslator::Axis::VERTICAL, "mousePosVert", "mouseMoveVert");

	translator->addMouseButtonMapping(InputTranslator::MouseButton::LEFT, "gogogogo");
	translator->addMouseButtonMapping(InputTranslator::MouseButton::MIDDLE, "rollMe!");

	m_InputQueue.init(std::move(translator));

	m_Network = INetwork::createNetwork();
	m_Network->setLogFunction(&Logger::logRaw);
	m_Network->initialize();

	m_EventManager.reset(new EventManager());

	m_EventManager->addListener(EventListenerDelegate(&m_InputQueue, &Input::lockMouse), MouseEventDataLock::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &BaseGameApp::showMouse), MouseEventDataShow::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &BaseGameApp::startGame), GameStartedEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &BaseGameApp::gameLeft), GameLeftEventData::sk_EventType);
	m_EventManager->addListener(EventListenerDelegate(this, &BaseGameApp::quitGame), QuitGameEventData::sk_EventType);

	m_GameLogic.reset(new GameLogic());
	m_SceneManager.init(m_Graphics, m_ResourceManager.get(), &m_InputQueue, m_GameLogic.get(), m_EventManager.get());
					
	m_MemoryInfo.update();
	
	m_ActorFactory.setPhysics(m_Physics);
	m_ActorFactory.setEventManager(m_EventManager.get());
	m_ActorFactory.setResourceManager(m_ResourceManager.get());

	m_GameLogic->initialize(m_ResourceManager.get(), m_Physics, &m_ActorFactory, m_EventManager.get(), m_Network);

	// Set Current Size
	m_NewWindowSize = m_Window.getSize();
}

void BaseGameApp::run()
{
	Logger::log(Logger::Level::INFO, "Running the game");

	m_ShouldQuit = false;

	resetTimer();

#ifdef _DEBUG
	m_GameLogic->playLocalLevel();
#endif

	while (!m_ShouldQuit)
	{
		Logger::log(Logger::Level::TRACE, "New frame");

		m_InputQueue.onFrame();
		m_Window.pollMessages();

		handleInput();

		updateLogic();

		render();
		
		m_MemoryInfo.update();
		updateDebugInfo();
	}
}

void BaseGameApp::shutdown()
{
	Logger::log(Logger::Level::INFO, "Shutting down the game app");
	
	m_GameLogic->shutdown();
	m_GameLogic.reset();

	INetwork::deleteNetwork(m_Network);	
	m_Network = nullptr;
	
	m_SceneManager.destroy();

	m_ResourceManager.reset();

	m_InputQueue.destroy();
	
	IPhysics::deletePhysics(m_Physics);
	m_Physics = nullptr;

	ISound::deleteSound(m_Sound);
	m_Sound = nullptr;

	IGraphics::deleteGraphics(m_Graphics);
	m_Graphics = nullptr;

	m_EventManager.reset();

	m_Window.destroy();
}

std::string BaseGameApp::getGameTitle() const
{
	return m_GameTitle;
}

XMFLOAT2 BaseGameApp::getWindowSize() const
{
	// TODO: Read from user option
	
	const static XMFLOAT2 size = XMFLOAT2(1280, 720);
	return size;
}

bool BaseGameApp::handleWindowClose(WPARAM /*p_WParam*/, LPARAM /*p_LParam*/, LRESULT& p_Result)
{
	Logger::log(Logger::Level::DEBUG_L, "Handling window close");

	m_ShouldQuit = true;
	p_Result = 0;
	return true;
}
bool BaseGameApp::handleWindowExitSizeMove(WPARAM /*p_WParam*/, LPARAM p_LParam, LRESULT& p_Result)
{
	Logger::log(Logger::Level::DEBUG_L, "Handling window when the user releases the resize bars.");

	m_Window.setSize(m_NewWindowSize);				

	p_Result = 0;
	return true;
}
bool BaseGameApp::handleWindowSize(WPARAM p_WParam, LPARAM p_LParam, LRESULT& p_Result)
{
	Logger::log(Logger::Level::DEBUG_L, "Handling window when the user resizes the window.");

	m_NewWindowSize = DirectX::XMFLOAT2(LOWORD(p_LParam),HIWORD(p_LParam));

	switch(p_WParam)
	{
	case SIZE_MAXIMIZED:
		{
			m_Window.setSize(m_NewWindowSize);
			m_Window.setIsMaximized(true);
			p_Result = 0;
			return true;
		}
	case SIZE_MAXHIDE:{return false;}
	case SIZE_MAXSHOW:{return false;}
	case SIZE_MINIMIZED:{return false;}
	case SIZE_RESTORED:
		{
			if(m_Window.getIsMaximized())
			{
				m_Window.setIsMaximized(false);
				m_Window.setSize(m_NewWindowSize);
				p_Result = 0;
				return true;
			}
			return false;
		}
	default:
		return false;
	}
	
}

void BaseGameApp::updateDebugInfo()
{
	m_TimeToNextMemUpdate -= m_DeltaTime;
	if (m_TimeToNextMemUpdate > 0.f)
	{
		return;
	}

	m_TimeToNextMemUpdate = m_MemUpdateDelay;

	std::string test = std::to_string(m_MemoryInfo.getPhysicalMemoryUsage());
	std::string vMemUsage = "Virtual RAM usage: " + std::to_string(m_MemoryInfo.getVirtualMemoryUsage()) + "B";
	std::string pMemUsage = "Physical RAM usage: " + std::to_string(m_MemoryInfo.getPhysicalMemoryUsage()) + "B";
	std::string gMemUsage = "Video usage: " + std::to_string(m_Graphics->getVRAMUsage()) + "B";

	std::string speed = "DeltaTime: " + std::to_string(m_DeltaTime) + " FPS: " + std::to_string(1.0f/m_DeltaTime);

	m_Window.setTitle(getGameTitle() + " | " + vMemUsage + " | " + pMemUsage + " | " + gMemUsage + " | " + speed);
}

void BaseGameApp::resetTimer()
{
	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	m_SecsPerCnt = 1.0f / (float)cntsPerSec;

	m_PrevTimeStamp = 0;
	m_CurrTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTimeStamp);
	m_CurrTimeStamp--;
}

void BaseGameApp::updateTimer()
{
	m_PrevTimeStamp = m_CurrTimeStamp;
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTimeStamp);
	m_DeltaTime = ((m_CurrTimeStamp - m_PrevTimeStamp) * m_SecsPerCnt);// / 10.0f; // To debug the game at low refreshrate.
	static const float maxDeltaTime = 1.f / 24.f; // Up from 5.f. Animations start behaving wierd if frame rate drops below 24. 
	if (m_DeltaTime > maxDeltaTime)
	{
		Logger::log(Logger::Level::WARNING, "Computer to slow or something");
		m_DeltaTime = maxDeltaTime;
	}
}

void BaseGameApp::handleInput()
{
	for (auto& in : m_InputQueue.getFrameInputs())
	{
		std::ostringstream msg;
		msg << "Received input action: " << in.m_Action << " (" << std::setprecision(2) << std::fixed << in.m_Value << ")";
		Logger::log(Logger::Level::TRACE, msg.str());

		// Pass keystrokes to all active scenes.
		m_SceneManager.registeredInput(in.m_Action, in.m_Value, in.m_PrevValue);
	}
}

void BaseGameApp::updateLogic()
{
	updateTimer();

	m_SceneManager.onFrame(m_DeltaTime);
	m_GameLogic->onFrame(m_DeltaTime);

	m_EventManager->processEvents();
}

void BaseGameApp::render()
{
	m_SceneManager.render();
	m_Graphics->drawFrame();
}

void BaseGameApp::startGame(IEventData::Ptr p_Data)
{
	m_SceneManager.startRun();
}

void BaseGameApp::gameLeft(IEventData::Ptr p_Data)
{
	std::shared_ptr<GameLeftEventData> data = std::static_pointer_cast<GameLeftEventData>(p_Data);

	if (data->getGoBack())
	{
		m_SceneManager.startMenu();
	}
	else
	{
		m_SceneManager.gotoPostGame();
	}
}

void BaseGameApp::quitGame(IEventData::Ptr p_Data)
{
	m_ShouldQuit = true;
}

void BaseGameApp::showMouse(IEventData::Ptr p_Data)
{
	std::shared_ptr<MouseEventDataShow> data = std::static_pointer_cast<MouseEventDataShow>(p_Data);
	m_Window.setShowCursor(data->getShowState());
}
