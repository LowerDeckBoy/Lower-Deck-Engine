#include "Application.hpp"
#include <ImGui/imgui_impl_win32.h>

namespace mf
{
	App::App(HINSTANCE hInstance, WindowParameters StartUp)
		: Window(hInstance, StartUp)
	{
		m_AppTimer = std::make_unique<Timer>();
	}

	App::~App()
	{
	}

	void App::Initialize()
	{
		Window::Create();

		
		m_Gfx = std::make_shared<RHI::D3D12Context>();

		//m_RHI = std::make_unique<RHI::D3D12Context>();

		m_ActiveScene = std::make_unique<Scene>(Window::Width, Window::Height, m_Gfx.get());
		m_Renderer = std::make_unique<Renderer>(m_Gfx.get());
		m_Renderer->SetScene(m_ActiveScene.get());

#if EDITOR_MODE
		m_Editor = std::make_unique<editor::Editor>(m_Gfx.get(), m_Renderer.get(), m_AppTimer.get());
		m_Editor->SetScene(m_ActiveScene.get());
#endif

		m_ActiveScene->AddModel("Assets/Models/sponza/Sponza.gltf");
		//m_ActiveScene->AddModel("Assets/Models/cube/Cube.gltf");
		//m_ActiveScene->AddModel("Assets/Models/Bistro-gltf/BistroExterior.gltf");
		//m_ActiveScene->AddModel("Assets/Models/scifi_girl/scene.gltf");
		//m_ActiveScene->AddModel("Assets/Models/nara/scene.gltf");
		//m_ActiveScene->AddModel("Assets/Models/DamagedHelmet/DamagedHelmet.gltf");
		//m_ActiveScene->AddModel("Assets/Models/SciFiHelmet/SciFiHelmet.gltf");

		
		m_Gfx->ExecuteCommandList(m_Gfx->GraphicsCommandList, m_Gfx->Device->GetGfxQueue(), false);
	}

	void App::Run()
	{
		Window::OnShow();
		
		::MSG msg{};
		m_AppTimer->Reset();

		while (!bShouldQuit)
		{
			if (msg.message == WM_QUIT)
				bShouldQuit = true;

			if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			m_AppTimer->Tick();
			m_AppTimer->GetFrameStats();
			
			// Render
			if (!bAppPaused)
			{
				
				m_ActiveScene->GetCamera()->ProcessInputs(m_AppTimer->DeltaTime());

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
				m_ActiveScene->m_Camera->Update();
				
			}
		}

		m_Gfx->Device->IdleGPU();
	}

	void App::OnResize()
	{
		m_Renderer->OnResize(Window::Width, Window::Height);
		m_ActiveScene->OnResize(static_cast<float>((float)Window::Width / (float)Window::Height));
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
		extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
		if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
			return true;
#endif

		switch (Msg)
		{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				bAppPaused = true;
				m_AppTimer->Stop();
			}
			else
			{
				bAppPaused = false;
				m_AppTimer->Start();
			}
		
			return 0;
		}
		case WM_SIZE:
		{
			Window::Width = static_cast<unsigned int>(LOWORD(lParam));
			Window::Height = static_cast<unsigned int>(HIWORD(lParam));
			Window::AspectRatio = static_cast<float>(Window::Width) / static_cast<float>(Window::Height);

			//if (!m_Renderer)
			//{
			//	//LOG_CRITICAL("Renderer not found!\n");
			//	return 0;
			//}
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
			m_AppTimer->Stop();

			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			bAppPaused = false;
			bIsResizing = false;
			m_AppTimer->Start();

			return 0;
		}
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				::PostQuitMessage(0);
			}
			return 0;
		case WM_QUIT:
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}

		return ::DefWindowProc(hWnd, Msg, wParam, lParam);
	}

}
