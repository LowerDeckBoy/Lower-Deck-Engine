#pragma once

/*
	Editor/Components.hpp
	
*/

#include <Engine/Scene/Components/LightComponent.hpp>

namespace lde
{
	class SceneLighting;
}

namespace lde::editor
{
	extern void DrawFloat3(std::string_view Label, DirectX::XMFLOAT3& Float3, float ResetValue = 0.0f);

	extern void DrawColorEdit(std::string_view Label, DirectX::XMFLOAT4& Float4);
	
	extern void DrawPointLight(std::string_view Label, PointLightComponent& LightComponent);

} // namespace lde::editor
