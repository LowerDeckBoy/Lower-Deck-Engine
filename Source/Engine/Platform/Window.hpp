#pragma once

#include "Platform.hpp"
#include <Core/String.hpp>

namespace lde
{
	struct WindowParameters
	{
		unsigned int Width;
		unsigned int Height;

		bool bMaximize;
	};

	class Window
	{
	public:
		Window(WindowParameters StartUpParameters);
		virtual ~Window();

		void Create();

		void OnShow();

		void Release();

		virtual LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) = 0;

		inline static unsigned int Width	= 1280;
		inline static unsigned int Height	= 720;
		inline static float AspectRatio		= static_cast<float>(Width) / static_cast<float>(Height);

		static void OnCursorShow();
		static void OnCursorHide();

		static bool bCursorVisible;
		static bool bShouldQuit;

		static HINSTANCE GetHInstance()
		{
			return s_hInstance;
		}

		static HWND GetHWnd()
		{
			return s_hWnd;
		}

		std::string GetTitle() const
		{
			return m_Title;
		}
		
	protected:
		static HINSTANCE s_hInstance;
		static HWND s_hWnd;

		RECT m_WindowRect{};

		LPCWSTR m_WindowClass	= L"Lower Deck Engine";
		std::string m_Title		= "Lower Deck Engine";

		WindowParameters m_Parameters{};

	public:
		// Managing application states via Message Procedures
		static bool bAppPaused;
		static bool bMinimized;
		static bool bMaximized;
		static bool bIsResizing;

	};
}
