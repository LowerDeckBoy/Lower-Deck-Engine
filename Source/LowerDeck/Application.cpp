#include "Application.hpp"
#include <ImGui/imgui_impl_win32.h>

#include <Engine/Scene/SceneLoader.hpp>

#if EDITOR_MODE
	extern LRESULT IMGUI_API ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

namespace lde
{
	App::App(WindowParameters StartUp)
		: Window(StartUp)
	{

	}

	App::~App() = default;

	void App::Initialize()
	{
		Window::Create();
		
		m_Gfx = std::make_unique<D3D12RHI>();

		m_ActiveScene = std::make_unique<Scene>(Window::Width, Window::Height, m_Gfx.get());
		m_Renderer = std::make_unique<Renderer>(m_Gfx.get(), m_ActiveScene.get());

#if EDITOR_MODE
		m_Editor = std::make_unique<editor::Editor>(m_Gfx.get(), m_Renderer.get(), &m_AppTimer);
		m_Editor->SetScene(m_ActiveScene.get());
#endif
		
		SceneLoader::Load(m_Gfx.get(), m_ActiveScene.get(), "sample_scene.json");
		//SceneLoader::Load(m_Gfx.get(), m_ActiveScene.get(), "sample_scene2.json");

		m_Gfx->Device->ExecuteCommandList(CommandType::eGraphics, false);
	}
	
	void App::Run()
	{
		Window::OnShow();
		
		m_AppTimer.Reset();

		while (!bShouldQuit)
		{
			Window::ProcessMessages();

			m_AppTimer.Tick();

			// Render
			if (!bAppPaused)
			{
				m_AppTimer.GetFrameStats();
				m_ActiveScene->GetCamera()->ProcessInputs(m_AppTimer.DeltaTime());
				m_ActiveScene->Camera->Update();

#if EDITOR_MODE
				m_Editor->OnBeginFrame();
				m_Renderer->Update();
				m_Renderer->Render();
				m_Editor->OnEndFrame();
#else
				m_Renderer->Update();
				m_Renderer->Render();
#endif
				m_Renderer->Present();
			}
		}

		m_Gfx->Device->IdleGPU();
	}

	void App::OnResize()
	{
		m_Renderer->OnResize(Window::Width, Window::Height);
		m_ActiveScene->OnResize(static_cast<float>((float)Window::Width / Window::Height));
	}

	void App::Release()
	{
#if EDITOR_MODE
		m_Editor.reset();
#endif
		m_ActiveScene.reset();
		m_Renderer.reset();
		m_Gfx.reset();
		Window::Release();
	}



	LRESULT App::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
#if EDITOR_MODE
		
		if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		{
			return true;
		}
#endif

		switch (Msg)
		{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				bAppPaused = true;
				m_AppTimer.Stop();
			}
			else
			{
				bAppPaused = false;
				m_AppTimer.Start();
			}
			
			return 0;
		}
		case WM_SIZE:
		{
			Window::Width = static_cast<unsigned int>(LOWORD(lParam));
			Window::Height = static_cast<unsigned int>(HIWORD(lParam));
			Window::AspectRatio = static_cast<float>(Window::Width) / static_cast<float>(Window::Height);

			if (wParam == SIZE_MINIMIZED)
			{
				bAppPaused = true;
				bMinimized = true;
				bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				bAppPaused = false;
				bMinimized = false;
				bMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (bMinimized)
				{
					bAppPaused = false;
					bMinimized = false;
				}
				else if (bMaximized)
				{
					bAppPaused = false;
					bMaximized = false;
				}
				OnResize();
			}

			return 0;
		}
		case WM_ENTERSIZEMOVE:
		{
			bAppPaused = true;
			bIsResizing = true;
			m_AppTimer.Stop();

			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			bAppPaused = false;
			bIsResizing = false;
			m_AppTimer.Start();

			return 0;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = 800;
			lpMMI->ptMinTrackSize.y = 640;
			return 0;
		}
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				::PostQuitMessage(0);
			}
			return 0;
		case WM_QUIT:
			[[fallthrough]];
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, Msg, wParam, lParam);
	}
} // namespace lde
