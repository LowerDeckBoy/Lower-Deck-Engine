#pragma once

/*

*/

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_win32.h>

namespace lde
{
	class Window;
	class D3D12RHI;
	class Renderer;
	class Timer;
	class Scene;
	class Entity;
}

namespace lde::editor
{
	enum class Theme
	{
		eDark,
		eLight
	};

	class Editor
	{
	public:
		Editor(RHI::D3D12RHI* pGfx, Renderer* pRenderer, Timer* pTimer);
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
	
	private:
		void Initialize(RHI::D3D12RHI* pGfx, Timer* pTimer);
		void Release();
	
		void DrawNode(Entity& Entity);
	
		void DrawComponentsData(Entity& Entity);
		template<typename T, typename UI>
		void DrawProperties(Entity& Entity, UI ui);
	
		void DrawFloat3(std::string Label, DirectX::XMFLOAT3& Float3, float ResetValue = 0.0f);
	
		void PrintLogs();
		void ClearLogs();

	private:
		RHI::D3D12RHI*	m_Gfx	= nullptr;
		Timer*			m_Timer	= nullptr;
		Renderer*		m_Renderer = nullptr;
		Scene*			m_ActiveScene = nullptr;
	
		Entity m_SelectedEntity;
	
		ImGuiStyle*		m_EditorStyle = nullptr;
		ImFont*			m_EditorFont = nullptr;
		ImGuiViewport*	m_EditorViewport = nullptr;
	
		Theme m_CurrentTheme{ Theme::eDark };
	
		static bool bSceneOnly;
	};

} // namespace lde::editor
