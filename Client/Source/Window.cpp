#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>

#include "Window.h"
#include "MyExceptions.h"

const LPCWSTR Window::m_ClassName = L"Havenborough_Game_Client";
std::vector<std::pair<HWND, Window*>> Window::m_CallbackWindows;
int Window::m_ClassUseCount = 0;

Window::Window()
	: m_Title("Window title: Change me!"),
	  m_Handle(NULL),
	  m_Icon(NULL),
	  m_ShowingCursor(true),
	  m_IsVisible(true),
	  m_Initialized(false)
{
	m_Size.x = 800;
	m_Size.y = 480;
}

Window::~Window()
{
	if (m_Initialized)
	{
		destroy();
		m_Initialized = false;
	}
}

void Window::init(const std::string& p_Title, UVec2 p_WindowSize)
{
	m_Title = p_Title;
	m_Size = p_WindowSize;

	HMODULE hInstance = GetModuleHandleW(NULL);

	if (m_ClassUseCount == 0)
	{
		WNDCLASSEXW windowClassDescription;
		windowClassDescription.cbSize			= sizeof(WNDCLASSEXW);
		windowClassDescription.style			= 0;
		windowClassDescription.lpfnWndProc		= windowProc;
		windowClassDescription.cbClsExtra		= 0;
		windowClassDescription.cbWndExtra		= 0;
		windowClassDescription.hInstance		= hInstance;
		windowClassDescription.hIcon			= m_Icon;
		windowClassDescription.hCursor			= LoadCursor(NULL, IDC_ARROW);
		windowClassDescription.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		windowClassDescription.lpszMenuName		= NULL;
		windowClassDescription.lpszClassName	= m_ClassName;
		windowClassDescription.hIconSm			= NULL;

		if (!RegisterClassExW(&windowClassDescription))
		{
			//throw WindowException("Window could not be registered", __LINE__, __FILE__);
		}

		m_ClassUseCount++;
	}

	RECT windowSize = { 0, 0, p_WindowSize.x, p_WindowSize.y };
	AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);

	std::vector<wchar_t> convertedTitle(p_Title.size() + 1);
	mbstowcs(convertedTitle.data(), p_Title.data(), p_Title.size() + 1);
	m_Handle = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW,
		m_ClassName,
		convertedTitle.data(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowSize.right - windowSize.left,
		windowSize.bottom - windowSize.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (m_Handle == NULL)
	{
		const DWORD error = GetLastError();
		throw WindowException("Window could not be created: " + std::to_string(error), __LINE__, __FILE__);
	}

	if (m_IsVisible)
	{
		ShowWindow(m_Handle, SW_SHOWDEFAULT);
	}
	else
	{
		ShowWindow(m_Handle, SW_HIDE);
	}

	if (!m_ShowingCursor)
	{
		ShowCursor(FALSE);
	}

	m_CallbackWindows.push_back(std::make_pair(m_Handle, this));

	m_Initialized = true;
}

void Window::destroy()
{
	m_RegisteredCallbacks.clear();

	if (m_Handle != NULL)
	{
		if (!DestroyWindow(m_Handle))
		{
			throw WindowException("Window could not be destroyed", __LINE__, __FILE__);
		}

		m_ClassUseCount--;
		if (m_ClassUseCount == 0)
		{
			UnregisterClassW(m_ClassName, GetModuleHandleW(NULL));
		}

		for (size_t i = 0; i < m_CallbackWindows.size() - 1; i++)
		{
			if (m_CallbackWindows[i].first == m_Handle)
			{
				std::swap(m_CallbackWindows[i], m_CallbackWindows.back());
				m_CallbackWindows.pop_back();
			}
		}

		m_Handle = NULL;
	}

	m_Initialized = false;
}

void Window::pollMessages()
{
	MSG message = {};
	while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

void Window::registerCallback(UINT p_MessageType, callbackFunc_t p_Callback)
{
	m_RegisteredCallbacks.push_back(std::make_pair(p_MessageType, p_Callback));
}

HWND Window::getHandle() const
{
	return m_Handle;
}

HICON Window::getIcon() const
{
	return m_Icon;
}

void Window::setIcon(HICON p_Icon)
{
	if (m_Icon != p_Icon)
	{
		SetClassLongPtrW(m_Handle, GCLP_HICON, (LONG)p_Icon);
		m_Icon = p_Icon;
	}
}

UVec2 Window::getSize() const
{
	return m_Size;
}

void Window::setSize(UVec2 p_NewSize)
{
	if (m_Size.x != p_NewSize.x || m_Size.y != p_NewSize.y)
	{
		RECT windowSize = { 0, 0, p_NewSize.x, p_NewSize.y };
		AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);

		if (!SetWindowPos(m_Handle, HWND_TOP, 0, 0, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, SWP_NOMOVE | SWP_NOZORDER))
		{
			throw WindowException("Window could not be resized", __LINE__, __FILE__);
		}

		m_Size = p_NewSize;
	}
}

bool Window::getShowCursor() const
{
	return m_ShowingCursor;
}

void Window::setShowCursor(bool p_Show)
{
	if (m_ShowingCursor != p_Show)
	{
		ShowCursor(p_Show);

		m_ShowingCursor = p_Show;
	}
}

std::string Window::getTitle() const
{
	return m_Title;
}

void Window::setTitle(const std::string& p_NewTitle)
{
	if (m_Title != p_NewTitle)
	{
		std::vector<wchar_t> convertedTitle(p_NewTitle.size() + 1);
		mbstowcs(convertedTitle.data(), p_NewTitle.data(), p_NewTitle.size() + 1);

		if (!SetWindowTextW(m_Handle, convertedTitle.data()))
		{
			throw WindowException("Could not set title of window", __LINE__, __FILE__);
		}

		m_Title = p_NewTitle;
	}
}

bool Window::getIsVisible() const
{
	return m_IsVisible;
}

void Window::setIsVisible(bool p_Visible)
{
	if (m_IsVisible != p_Visible)
	{
		if (p_Visible)
		{
			ShowWindow(m_Handle, SW_SHOWNORMAL);
		}
		else
		{
			ShowWindow(m_Handle, SW_HIDE);
		}

		m_IsVisible = p_Visible;
	}
}

LRESULT CALLBACK Window::windowProc(_In_ HWND p_Hwnd, _In_ UINT p_UMsg, _In_ WPARAM p_WParam, _In_ LPARAM p_LParam)
{
	LRESULT result;

	if (p_Hwnd == NULL)
	{
		for (auto& window : m_CallbackWindows)
		{
			if (window.second->dispatchMessage(p_UMsg, p_WParam, p_LParam, result))
			{
				return result;
			}
		}
	}
	else
	{
		Window* calledWindow = nullptr;
		for (auto& window : m_CallbackWindows)
		{
			if (window.first == p_Hwnd)
			{
				calledWindow = window.second;
				break;
			}
		}

		if (calledWindow == nullptr)
		{
			return DefWindowProcW(p_Hwnd, p_UMsg, p_WParam, p_LParam);
		}

		if (calledWindow->dispatchMessage(p_UMsg, p_WParam, p_LParam, result))
		{
			return result;
		}
	}

	return DefWindowProcW(p_Hwnd, p_UMsg, p_WParam, p_LParam);
}

bool Window::dispatchMessage(UINT p_UMsg, WPARAM p_WParam, LPARAM p_LParam, LRESULT& p_Result)
{
	for (auto& callback : m_RegisteredCallbacks)
	{
		if (callback.first == p_UMsg)
		{
			if (callback.second(p_WParam, p_LParam, p_Result))
			{
				return true;
			}
		}
	}

	return false;
}