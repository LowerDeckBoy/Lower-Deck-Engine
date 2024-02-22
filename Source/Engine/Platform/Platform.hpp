#pragma once

// TEMPORAL
#if defined (_WIN64) || (_WINDOWS)
#	ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	include <dwmapi.h>
#endif
#include "Timer.hpp"

namespace lde
{
	enum class PlatformType
	{
		eWindows,
		eLinux
	};

#if defined (_WIN64) || (_WINDOWS)
#define PLATFORM_WIN64 1
#define PLATFORM_LINUX 0
	constexpr auto PLATFORM = PlatformType::eWindows;
#else
#define PLATFORM_WIN64 0
#define PLATFORM_LINUX 1
	constexpr auto PLATFORM = PlatformType::eLinux;
#endif

}
