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
	class EditorComponents
	{
	public:
		void DrawFloat3();

		void DrawDirectLight();

		/**
		 * @brief Draw all lights that are inside of SceneLighting object.
		 */
		void DrawLightData(SceneLighting* pSceneLighting);


	};
} // namespace lde::editor
