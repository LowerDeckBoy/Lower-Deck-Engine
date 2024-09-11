#include "Window.hpp"
#include <Core/CoreDefines.hpp>

#pragma comment(lib, "dwmapi")

namespace lde
{
	HINSTANCE Window::s_hInstance = nullptr;
	HWND Window::s_hWnd			= nullptr;

	bool Window::bShouldQuit	= false;
	bool Window::bCursorVisible = true;

	bool Window::bAppPaused		= false;
	bool Window::bMinimized		= false;
	bool Window::bMaximized		= false;
	bool Window::bIsResizing	= false;

	namespace { Window* window = 0; }
	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return window->WindowProc(hWnd, msg, wParam, lParam);
	}

	Window::Window(WindowParameters StartUpParameters)
	{
		s_hInstance = ::GetModuleHandleA(0);

		window = this;

		m_Parameters	= StartUpParameters;
		Width			= StartUpParameters.Width;
		Height			= StartUpParameters.Height;

		m_Title.append(std::format(" {}", ENGINE_VERSION));

	}

	Window::~Window()
	{
		Release();
	}

	void Window::Create()
	{

		if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
		{
			::MessageBoxA(nullptr, "Failed to set SetProcessDpiAwarenessContext!", "Warning", MB_OK);
		}

		::WNDCLASSEX wcex{};
		wcex.cbSize			= sizeof(::WNDCLASSEX);
		wcex.style			= CS_OWNDC;
		wcex.hInstance		= s_hInstance;
		wcex.lpszClassName	= m_WindowClass;
		wcex.lpfnWndProc	= MsgProc;
		// Blends to dark mode
		wcex.hbrBackground	= ::CreateSolidBrush(RGB(32, 32, 32));

		if (!::RegisterClassEx(&wcex))
		{
			::MessageBox(nullptr, L"Failed to register Window class!", L"Error", MB_OK);
			//throw std::exception();
		}

		m_WindowRect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
		::AdjustWindowRect(&m_WindowRect, WS_OVERLAPPEDWINDOW, false);
		
		const int width = static_cast<int>(m_WindowRect.right - m_WindowRect.left);
		const int height = static_cast<int>(m_WindowRect.bottom - m_WindowRect.top);

		s_hWnd = ::CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_NOREDIRECTIONBITMAP,
			m_WindowClass, String::ToWide(m_Title).c_str(), 
			WS_OVERLAPPEDWINDOW,
			0, 0, width, height, 
			nullptr, nullptr, 
			s_hInstance, this);

		// Centering window position upon first appearing
		const int xPos = (::GetSystemMetrics(SM_CXSCREEN) - m_WindowRect.right) / 2;
		const int yPos = (::GetSystemMetrics(SM_CYSCREEN) - m_WindowRect.bottom) / 2;

		::SetWindowPos(s_hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		BOOL bDarkMode = TRUE;
		::DwmSetWindowAttribute(s_hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));

		const MARGINS shadows{ 1, 1, 1, 1 };
		::DwmExtendFrameIntoClientArea(s_hWnd, &shadows);

	}

	void Window::Release()
	{
		::UnregisterClass(m_WindowClass, s_hInstance);

		if (s_hWnd)		 
		{
			s_hWnd = nullptr;
		}

		if (s_hInstance)
		{
			s_hInstance = nullptr;
		}
	}

	void Window::OnShow()
	{
		::ShowWindow(s_hWnd, (m_Parameters.bMaximize) ? SW_SHOWMAXIMIZED : SW_SHOW);
		::SetForegroundWindow(s_hWnd);
		::SetFocus(s_hWnd);
		::UpdateWindow(s_hWnd);
	}

	void Window::OnCursorShow()
	{
		while (::ShowCursor(true) < 0)
		{
			bCursorVisible = true;
		}
	}

	void Window::OnCursorHide()
	{
		while (::ShowCursor(false) >= 0)
		{
			bCursorVisible = false;
		}
	}

}
