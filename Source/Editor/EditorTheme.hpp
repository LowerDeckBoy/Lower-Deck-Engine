#pragma once
#include "Colors.hpp"
#include <Engine/Platform/Window.hpp>
#include <dwmapi.h>

namespace lde::editor
{
	// Note:
	// Window caption color needs to be change in both Themes.
	// Otherwise, switching between themes leaves only one color.

	struct EditorThemes
	{
		static void DarkTheme(ImGuiStyle& InStyle)
		{
			const BOOL bDarkMode = TRUE;
			::DwmSetWindowAttribute(lde::Window::GetHWnd(), DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));
		
			COLORREF captionColor{ RGB(32, 32, 32) };
			::DwmSetWindowAttribute(lde::Window::GetHWnd(), DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
		
			ImGui::StyleColorsDark();
			InStyle.WindowRounding		= 0.0f;
			InStyle.WindowBorderSize	= 0.0f;
			InStyle.FrameRounding		= 1.0f;
		
			InStyle.Colors[ImGuiCol_Text]					= Colors::White;
			InStyle.Colors[ImGuiCol_WindowBg]				= Colors::DarkBackground;
			InStyle.Colors[ImGuiCol_MenuBarBg]				= Colors::DarkBackground;
			InStyle.Colors[ImGuiCol_TitleBg]				= Colors::DarkBackground;
			InStyle.Colors[ImGuiCol_DockingEmptyBg]			= Colors::DarkBackground;
			InStyle.Colors[ImGuiCol_TitleBgActive]			= Colors::DarkBackgroundActive;
			InStyle.Colors[ImGuiCol_Border]					= Colors::DarkBackgroundActive;
			InStyle.Colors[ImGuiCol_BorderShadow]			= Colors::BorderShadow;
		
			InStyle.Colors[ImGuiCol_Tab]					= Colors::Gray;	
			InStyle.Colors[ImGuiCol_TabActive]				= Colors::Coral;
			InStyle.Colors[ImGuiCol_TabHovered]				= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_TabUnfocused]			= Colors::Gray;
			InStyle.Colors[ImGuiCol_TabUnfocusedActive]		= Colors::Gray;
		
			InStyle.Colors[ImGuiCol_Header]					= Colors::Coral;
			InStyle.Colors[ImGuiCol_HeaderActive]			= Colors::CoralActive;
			InStyle.Colors[ImGuiCol_HeaderHovered]			= Colors::Coral;
			InStyle.Colors[ImGuiCol_Button]					= Colors::Coral;
			InStyle.Colors[ImGuiCol_FrameBg]				= Colors::Coral;
		
			InStyle.Colors[ImGuiCol_DockingPreview]			= Colors::Coral;
			InStyle.Colors[ImGuiCol_ResizeGrip]				= Colors::Coral;
			InStyle.Colors[ImGuiCol_ResizeGripHovered]		= Colors::Coral;
			InStyle.Colors[ImGuiCol_FrameBgHovered]			= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_FrameBgActive]			= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_SeparatorHovered]		= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_ButtonHovered]			= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_ButtonActive]			= Colors::CoralActive;
			InStyle.Colors[ImGuiCol_TitleBgCollapsed]		= Colors::CoralHover;
		
			InStyle.Colors[ImGuiCol_SliderGrabActive]		= Colors::Red;
			
		
			InStyle.SeparatorTextBorderSize = 0.25f;
			InStyle.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
		
		}

		static void LightTheme(ImGuiStyle& InStyle)
		{
			const BOOL bDarkMode = FALSE;
			::DwmSetWindowAttribute(lde::Window::GetHWnd(), DWMWA_USE_IMMERSIVE_DARK_MODE, &bDarkMode, sizeof(bDarkMode));
			
			COLORREF captionColor = RGB(255, 236, 225);
			::DwmSetWindowAttribute(lde::Window::GetHWnd(), DWMWINDOWATTRIBUTE::DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
		
			ImGui::StyleColorsLight();
		
			InStyle.WindowRounding		= 0.0f;
			InStyle.WindowBorderSize	= 0.0f;
			InStyle.FrameRounding		= 1.0f;
		
			InStyle.Colors[ImGuiCol_Text]				= Colors::Black;
			InStyle.Colors[ImGuiCol_WindowBg]			= Colors::LightBackground;
			InStyle.Colors[ImGuiCol_MenuBarBg]			= Colors::LightBackground;
			InStyle.Colors[ImGuiCol_TitleBg]			= Colors::LightBackground;
			InStyle.Colors[ImGuiCol_DockingEmptyBg]		= Colors::LightBackground;
			InStyle.Colors[ImGuiCol_TitleBgActive]		= Colors::DarkBackgroundActive;
			InStyle.Colors[ImGuiCol_Border]				= Colors::DarkBackgroundActive;
			InStyle.Colors[ImGuiCol_BorderShadow]		= Colors::BorderShadow;
		
			InStyle.Colors[ImGuiCol_Header]				= Colors::Coral;
			InStyle.Colors[ImGuiCol_HeaderActive]		= Colors::CoralActive;
			InStyle.Colors[ImGuiCol_HeaderHovered]		= Colors::Coral;
			InStyle.Colors[ImGuiCol_Button]				= Colors::Coral;
			InStyle.Colors[ImGuiCol_FrameBg]			= Colors::Coral;
		
			InStyle.Colors[ImGuiCol_DockingPreview]		= Colors::Coral;
			InStyle.Colors[ImGuiCol_ResizeGrip]			= Colors::Coral;
			InStyle.Colors[ImGuiCol_Tab]				= Colors::Gray;
			InStyle.Colors[ImGuiCol_TabActive]			= Colors::Coral;
			InStyle.Colors[ImGuiCol_ResizeGripHovered]	= Colors::Coral;
			InStyle.Colors[ImGuiCol_FrameBgHovered]		= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_FrameBgActive]		= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_SeparatorHovered]	= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_TabHovered]			= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_ButtonHovered]		= Colors::CoralHover;
			InStyle.Colors[ImGuiCol_ButtonActive]		= Colors::CoralActive;
			InStyle.Colors[ImGuiCol_TitleBgCollapsed]	= Colors::CoralHover;
		
			InStyle.Colors[ImGuiCol_SliderGrabActive] = Colors::Red;
		
			InStyle.SeparatorTextBorderSize = 0.25f;
			InStyle.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
		
		}
		
	};

} // namespace lde::editor
