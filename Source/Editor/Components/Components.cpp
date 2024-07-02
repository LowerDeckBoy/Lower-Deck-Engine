#include <Engine/Core/String.hpp>
#include "Components.hpp"
#include <ImGui/imgui_internal.h>
#include "../EditorColors.hpp"

namespace lde::editor
{
	void DrawFloat3(std::string_view Label, DirectX::XMFLOAT3& Float3, float ResetValue)
	{
		if (ImGui::BeginTable("XYZ", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable))
		{
			ImGui::PushID(Label.data());
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(Label.data());
			ImGui::TableNextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });
			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 1.25f);
			// X
			{
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::Red);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
				if (ImGui::Button("X"))
					Float3.x = ResetValue;

				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				ImGui::DragFloat("##X", &Float3.x);
				ImGui::PopItemWidth();
				ImGui::SameLine();
			}
			// Y
			{
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::Green);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
				if (ImGui::Button("Y"))
					Float3.y = ResetValue;

				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				ImGui::DragFloat("##Y", &Float3.y);
				ImGui::PopItemWidth();
				ImGui::SameLine();
			}
			// Z
			{
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::Blue);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
				if (ImGui::Button("Z"))
					Float3.z = ResetValue;

				ImGui::PopStyleColor(3);

				ImGui::SameLine();
				ImGui::DragFloat("##Z", &Float3.z);
				ImGui::PopItemWidth();
			}

			ImGui::PopStyleVar();
			ImGui::PopID();
			ImGui::EndTable();
		}

		ImGui::Separator();
	}

	void DrawColorEdit(std::string_view Label, DirectX::XMFLOAT4& Float4)
	{
		float* color[4] = { &Float4.x, &Float4.y, &Float4.z, &Float4.w };
		ImGui::Text(Label.data());
		ImGui::SameLine();
		ImGui::ColorEdit4("##Label", *color, ImGuiColorEditFlags_NoBorder);

		//ImGui::Separator();
	}

	void DrawPointLight(std::string_view Label, PointLightComponent& LightComponent)
	{
		DrawFloat3("Position", LightComponent.Position);
		if (ImGui::BeginTable("XYZ", 2, ImGuiTableFlags_Resizable)) // ImGuiTableFlags_BordersInner |
		{
			ImGui::PushID(Label.data());
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 90.0f);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::Text("Visibility");
			ImGui::TableNextColumn();
			ImGui::SliderFloat("##Visibility", &LightComponent.Visibility, 0.0f, 1.0f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Ambient");

			ImGui::TableNextColumn();
			float* color[4] = { &LightComponent.Ambient.x, &LightComponent.Ambient.y, &LightComponent.Ambient.z, &LightComponent.Ambient.w };
			ImGui::ColorEdit4("##Ambient", *color, ImGuiColorEditFlags_NoBorder);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Range");
			ImGui::TableNextColumn();
			ImGui::DragFloat("##Range", &LightComponent.Range, 1.0f, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::PopID();
			ImGui::EndTable();
			ImGui::Separator();
		}
	}

} //namespace lde::editor

