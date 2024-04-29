#pragma once

// TEMPORAL
#if defined (_WIN64) || (_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	#include <dwmapi.h>
#endif
#include "Timer.hpp"

namespace lde
{
#if defined (_WIN64) || (_WINDOWS)
	#define PLATFORM_WIN64 1
#else
	#define PLATFORM_WIN64 0
#endif


} // namespace lde
