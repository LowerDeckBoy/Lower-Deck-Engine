// Forwards
#include <Engine/Platform/Window.hpp>
#include <Engine/RHI/D3D12/D3D12RHI.hpp>
#include <Engine/Render/Renderer.hpp>
#include <Engine/Scene/Scene.hpp>
#include "Editor.hpp"
#include "EditorColors.hpp"
#include "EditorTheme.hpp"
#include "Components/Components.hpp"
#include <Engine/RHI/RHICommon.hpp>
#include <ImGui/imgui_internal.h>
#include <Engine/Core/Logger.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/Scene/Components/Components.hpp>
#include <Engine/Scene/Components/CameraComponent.hpp>
#include <FontAwesome/IconsFontAwesome6.h>

namespace lde::editor 
{
	//using namespace RHI;

	bool Editor::bSceneOnly = true;

	constexpr auto EDITOR_FONT		= "Assets/Fonts/CascadiaCode-SemiBold.ttf";
	constexpr auto ICONS_FONT		= "Assets/Fonts/fa-solid-900.ttf";

	Editor::Editor(D3D12RHI* pGfx, Renderer* pRenderer, Timer* pTimer)
		: m_Gfx(pGfx), m_Renderer(pRenderer), m_Timer(pTimer)
	{
		Initialize(pGfx, pTimer);
	}

	Editor::~Editor()
	{
		Release();
	}

	void Editor::Initialize(D3D12RHI* pGfx, Timer* pTimer)
	{
		m_Gfx = pGfx;
		m_Timer = pTimer;

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		ImGuiIO& IO{ ImGui::GetIO() };
		m_EditorStyle = &ImGui::GetStyle();
		
		Themes::DarkTheme(*m_EditorStyle);

		// Enable Docking
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
		IO.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		IO.ConfigFlags	|= ImGuiConfigFlags_DockingEnable;
		IO.ConfigFlags	|= ImGuiConfigFlags_ViewportsEnable;
		
		//m_EditorHeap = std::make_unique<D3D12DescriptorHeap>(m_Gfx->Device.get(), HeapType::eSRV, 32768, "Editor SRV Heap");

		ImGui_ImplWin32_Init(lde::Window::GetHWnd());
		ImGui_ImplDX12_Init(m_Gfx->Device->GetDevice(),
			FRAME_COUNT,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_Gfx->Device->GetSRVHeap()->Get(),
			m_Gfx->Device->GetSRVHeap()->Get()->GetCPUDescriptorHandleForHeapStart(),
			m_Gfx->Device->GetSRVHeap()->Get()->GetGPUDescriptorHandleForHeapStart());

		// Main font
		{
			constexpr float fontSize{ 15.0f };
			m_EditorFont = IO.Fonts->AddFontFromFileTTF(EDITOR_FONT, fontSize);
		}
		
		// Icons font
		{
			constexpr float iconsSize{ 18 * 2.0f / 3.0f };
			static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
			ImFontConfig iconsConfig;
			iconsConfig.MergeMode = true;
			iconsConfig.PixelSnapH = true;
			iconsConfig.GlyphMinAdvanceX = iconsSize;
			iconsConfig.SizePixels = iconsSize;
			IO.Fonts->AddFontFromFileTTF(ICONS_FONT, iconsSize, &iconsConfig, iconsRanges);
		}

		m_EditorViewport = ImGui::GetMainViewport();
		m_EditorViewport->Flags |= ImGuiViewportFlags_TopMost;
		m_EditorViewport->Flags |= ImGuiViewportFlags_OwnedByApp;
		
		LOG_INFO("Editor: Editor layer initialized.");
	}

	void Editor::Release()
	{
		m_EditorFont = nullptr;
		m_EditorStyle = nullptr;
		m_EditorViewport = nullptr;

		m_Gfx = nullptr;

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		LOG_INFO("Editor: Editor layer released.");
	}

	void Editor::DrawNode(Entity& Entity)
	{
		if (!Entity.IsValid())
		{
			LOG_WARN("Invalid Entity");
			return;
		}

		auto& tag = m_ActiveScene->World()->Registry()->get<TagComponent>(Entity.ID());

		ImGuiTreeNodeFlags flags = ((m_SelectedEntity == Entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;
		bool opened = ImGui::TreeNodeEx((void*)(uint64)(uint32)Entity.ID(), flags, tag.Name.c_str());
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			m_SelectedEntity = Entity;
		}

		if (opened)
		{
			ImGui::Bullet();
			ImGui::Text("Extended");
			flags = ImGuiTreeNodeFlags_OpenOnArrow;
		
			ImGui::TreePop();
		}
	}

	void Editor::DrawComponentsData(Entity& Entity)
	{
		if (!Entity.HasComponent<TagComponent>())
		{
			return;
		}
		
		auto& tag{ Entity.GetComponent<TagComponent>() };
		ImGui::Text(tag.Name.c_str());
		ImGui::Separator();

		DrawProperties<CameraComponent>(Entity, [&](auto& Component)
			{
				DrawFloat3("Position", Component.Position);

				if (ImGui::BeginTable("Properties", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Speed");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##Speed", &Component.Speed, 1.0f, 1.0f, 500.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Sensivity");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##Sensivity", &Component.Sensivity, 0.1f, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Field of View");
					ImGui::TableNextColumn();
					ImGui::DragFloat("##Field of View", &Component.FoV, 0.1f, 1.0f, 90.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::EndTable();
				}

				if (ImGui::Button("Reset"))
				{
					Component.Reset();
				}
				ImGui::Separator();
			});

		DrawProperties<TransformComponent>(Entity, [&](auto& Component)
			{
				if (ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_DefaultOpen))
				{
					DrawFloat3("Position", Component.Translation);
					DrawFloat3("Rotation", Component.Rotation);
					DrawFloat3("Scale", Component.Scale, 1.0f);
					// TODO: Gotta add checking for update call
					Component.Update();

					if (ImGui::Button("Reset"))
						Component.Reset();
				}
			});

		DrawProperties<PointLightComponent>(Entity, [&](auto& Component)
			{
				DrawPointLight(tag.Name, Component);	
			});

		DrawProperties<DirectionalLightComponent>(Entity, [&](auto& Component)
			{
				DrawFloat3("Direction", Component.Direction);
				DrawColorEdit("Ambient", Component.Ambient);
				ImGui::Text("Visibility");
				ImGui::SameLine();
				ImGui::SliderFloat("##Visibility", &Component.Visibility, 0.0f, 1.0f);
				ImGui::Text("Casts shadows");
				ImGui::SameLine();
				ImGui::Checkbox("##Casts shadows", &Component.bCastShadows);

				if (Component.bCastShadows)
				{
					ImGui::Text("Shadow map lookup...");
					//ImGui::Image((ImTextureID))
				}
			});
	}

	template<typename T, typename UI>
	void Editor::DrawProperties(Entity& Entity, UI ui)
	{
		if (!Entity.HasComponent<T>())
			return;

		auto& component = Entity.GetComponent<T>();

		ui(component);
	}

	void Editor::OnBeginFrame()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX12_NewFrame();
		ImGui::NewFrame();

		ImGui::PushFont(m_EditorFont);

		m_EditorViewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(m_EditorViewport);
	}

	void Editor::OnEndFrame()
	{
		// Draw editor
		if (!bSceneOnly)
		{
			DrawMenuBar();
			DrawHierarchy();
			DrawProperty();
			DrawContent();
			DrawLogs();
			DrawScene();
		}
		else
		{
			DrawMenuBar();
			DrawScene();
		}
		
		// End editor
		ImGui::PopFont();
		ImGui::EndFrame();
		ImGui::Render();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		
		//m_Gfx->Device->GetGfxCommandList()->Get()->SetDescriptorHeaps(1, m_EditorHeap->GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_Gfx->Device->GetGfxCommandList()->Get());
	}

	void Editor::SetScene(Scene* pScene)
	{
		m_ActiveScene = pScene;
	}

	void Editor::DrawMenuBar()
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			ImGui::SeparatorText("File");
			if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN" Open", "Ctrl+O"))
			{

			}
			if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK" Save", "Ctrl+S"))
			{
				
			}

			ImGui::SeparatorText("Exit");
			if (ImGui::MenuItem(ICON_FA_POWER_OFF" Save and Exit"))
			{
			
			}
			if (ImGui::MenuItem(ICON_FA_POWER_OFF" Exit"))
			{
				::PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::SeparatorText("Editor");
			if (ImGui::BeginMenu(ICON_FA_PALETTE" Theme"))
			{
				if (ImGui::MenuItem(ICON_FA_MOON" Dark Theme"))
				{
					Themes::DarkTheme(*m_EditorStyle);
					LOG_INFO("Switched Editor Theme to Dark mode.");
				}
				if (ImGui::MenuItem(ICON_FA_SUN" Light Theme"))
				{
					Themes::LightTheme(*m_EditorStyle);
					LOG_INFO("Switched Editor Theme to Light mode.");
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem(ICON_FA_LANDMARK" Scene"))
			{

			}

			// Render Outputs; GBuffer etc
			if (ImGui::BeginMenu(ICON_FA_IMAGES" Render Target"))
			{
				if (ImGui::MenuItem("Shaded", nullptr, true))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eShaded;
				}
				else if (ImGui::MenuItem("Depth"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eDepth;
				}
				else if (ImGui::MenuItem("Base Color"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eBaseColor;
				}
				else if (ImGui::MenuItem("TexCoords"))
				{ 
					m_Renderer->SelectedRenderTarget = RenderOutput::eTexCoords;
				}
				else if (ImGui::MenuItem("Normal"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eNormal;
				}
				else if (ImGui::MenuItem("Metallic-Roughness"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eMetalRoughness;
				}
				else if (ImGui::MenuItem("Emissive"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eEmissive;
				}
				else if (ImGui::MenuItem("World Position"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eWorldPosition;
				}
				else if (ImGui::MenuItem("Raytracing"))
				{
					m_Renderer->SelectedRenderTarget = RenderOutput::eRaytracing;
				}
				
				ImGui::EndMenu();
			}

			ImGui::Checkbox("Scene only", &bSceneOnly);

			ImGui::EndMenu();
		}

		{
			ImGui::Separator();
			ImGui::Text("V-Sync");
			ImGui::Checkbox("##V-Sync", &Renderer::bVSync);

			ImGui::SetNextItemWidth(350.0f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 350.0f);
			ImGui::Text("%d FPS %.2f ms", m_Timer->FPS, m_Timer->Miliseconds);
			ImGui::SameLine();
			ImGui::Text("RAM: %.2fMB", Utility::MemoryUsage::ReadRAM());
			ImGui::SameLine();
			ImGui::Text("VRAM: %d MB", m_Gfx->QueryAdapterMemory());
			ImGui::SameLine();
	
		}
		
		ImGui::EndMainMenuBar();
		
	}

	void Editor::DrawScene()
	{
		ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);

		auto viewportSize{ ImGui::GetContentRegionAvail() };
		ImGui::Image((ImTextureID)m_Renderer->GetRenderTarget(), viewportSize);

		ImGui::End();
	}

	void Editor::DrawHierarchy()
	{
		ImGui::Begin("Hierarchy");

		auto view = m_ActiveScene->Registry()->view<TagComponent>();
		for (auto [entity, tag] : view.each())
		{
			Entity e(m_ActiveScene->World(), entity);
			DrawNode(e);
			ImGui::Separator();
		}

		// Scene Lighting here

		//auto light = m_ActiveScene->Registry()->view<PointLightComponent>();
		//for (auto [entity, comp] : light.each())
		//{
		//	Entity e(m_ActiveScene->World(), entity);
		//	DrawNode(e);
		//	ImGui::Separator();
		//}

		//m_ActiveScene->Registry()->view<DirectionalLightComponent>();
		//m_ActiveScene->Registry()->view<PointLightComponent>();
		//m_ActiveScene->Registry()->view<SpotLightComponent>();

		ImGui::End();
	}

	void Editor::DrawProperty()
	{
		ImGui::Begin("Properties");

		if (m_SelectedEntity.IsAlive())
		{
			DrawComponentsData(m_SelectedEntity);
		}

		ImGui::End();
	}

	void Editor::DrawContent()
	{
		ImGui::Begin("Content");

		ImGui::End();
	}

	void Editor::DrawLogs()
	{
		ImGui::Begin("Logs");
		ClearLogs();
		PrintLogs();
		ImGui::End();
	}

	void Editor::PrintLogs()
	{
		for (const auto& log : Logger::Logs)
		{
			ImGui::Text(log.c_str());
		}
	}

	void Editor::ClearLogs()
	{
		if (ImGui::Button("Clear logs"))
		{
			Logger::Logs.clear();
		}
	}

} // namespace lde::editor 
