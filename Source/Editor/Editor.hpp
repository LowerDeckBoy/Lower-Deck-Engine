#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>
#include <memory>

namespace lde
{
	class Window;
	class D3D12RHI;
	class D3D12DescriptorHeap;
	class Renderer;
	class Timer;
	class Scene;
	class Entity;
}

namespace lde::editor
{
	enum class EditorTheme
	{
		eDark,
		eLight
	};

	class Editor
	{
	public:
		Editor(D3D12RHI* pGfx, Renderer* pRenderer, Timer* pTimer);
		~Editor();
	
		/// @brief Called before Renderer BeginFrame()
		void OnBeginFrame();
	
		/// @brief Called before Renderer EndFrame()
		void OnEndFrame();
	
		void SetScene(Scene* pScene);
	
		void DrawMenuBar();
		void DrawScene();
		void DrawHierarchy();
		void DrawProperty();
		void DrawContent();
		void DrawLogs();

		void DrawSceneProperties();
	
	private:
		void Initialize(D3D12RHI* pGfx, Timer* pTimer);
		void Release();
	
		void DrawNode(Entity& Entity);
	
		void DrawComponentsData(Entity& Entity);
		template<typename T, typename UI>
		void DrawProperties(Entity& Entity, UI ui);
		
		void PrintLogs();
		void ClearLogs();

	private:
		D3D12RHI*	m_Gfx	= nullptr;
		Timer*			m_Timer	= nullptr;
		Renderer*		m_Renderer = nullptr;
		Scene*			m_ActiveScene = nullptr;
	
		Entity m_SelectedEntity;
	
		ImGuiStyle*		m_EditorStyle = nullptr;
		ImFont*			m_EditorFont = nullptr;
		ImGuiViewport*	m_EditorViewport = nullptr;
	
		EditorTheme m_CurrentTheme = EditorTheme::eDark;
	
		static bool bSceneOnly;

		std::unique_ptr<D3D12DescriptorHeap> m_EditorHeap;

	};

} // namespace lde::editor
