#pragma once

/*
	
*/

#include <Engine/Platform/Window.hpp>
#include <Engine/Core/CoreMinimal.hpp>

#include <Engine/RHI/D3D12/D3D12Context.hpp>
#include <Engine/RHI/D3D12/D3D12RHI.hpp>
#include <Engine/Render/Renderer.hpp>
#include <Engine/Scene/Scene.hpp>

#include <memory>

#define EDITOR_MODE 1

#if EDITOR_MODE
#	include <Editor/Editor.hpp>
#endif

namespace lde
{
	class App : public Window
	{
	public:
		App(HINSTANCE hInstance, WindowParameters StartUp);
		~App();

		void Initialize();
		void Run();
		void OnResize();
		void Release();

	protected:
		LRESULT WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) final;

	private:
		std::unique_ptr<Timer> m_AppTimer;

		std::shared_ptr<RHI::D3D12Context> m_Gfx;
		std::unique_ptr<RHI::RHI> m_RHI;
		std::unique_ptr<Renderer> m_Renderer;
		std::unique_ptr<Scene> m_ActiveScene;

#if EDITOR_MODE
		std::unique_ptr<editor::Editor> m_Editor;
#endif

	};
}
