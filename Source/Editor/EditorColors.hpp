#pragma once
#include <ImGui/imgui.h>

namespace mf::editor
{
	namespace Colors
	{
		/// @brief "Blends" Background and Menu Bar colors into Windows Dark theme.
		constexpr ImVec4 DarkBackground = ImVec4(0.125f, 0.125f, 0.125f, 1.0f);
	
		/// @brief 
		constexpr ImVec4 DarkBackgroundActive = ImVec4(0.155f, 0.155f, 0.155f, 1.0f);
	
		constexpr ImVec4 DarkHeader = ImVec4(0.097f, 0.097f, 0.097f, 1.0f);
	
		//constexpr ImVec4 LightBackground = ImVec4(1.0f, 0.855f, 0.855f, 1.0f);
		constexpr ImVec4 LightBackground = ImVec4(1.0f, 0.921f, 0.87f, 1.0f);
	
		/// @brief 
		constexpr ImVec4 BorderShadow = ImVec4(1.0f, 0.659f, 0.231f, 1.0f);
	
		/// @brief 
		constexpr ImVec4 White	= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr ImVec4 Black	= ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		constexpr ImVec4 Gray	= ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
	
		constexpr ImVec4 Red	= ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr ImVec4 Green	= ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr ImVec4 Blue	= ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	
		
		constexpr ImVec4 Coral			= ImVec4(1.0f, 0.329f, 0.431f, 1.0f);
		constexpr ImVec4 CoralHover		= ImVec4(1.0f, 0.0f, 0.3f, 1.0f);
		constexpr ImVec4 CoralActive	= ImVec4(1.0f, 0.0f, 0.5f, 1.0f);
	
	} // namespace Colors
} // namespace  mf::editor
