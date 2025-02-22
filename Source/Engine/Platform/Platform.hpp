#pragma once

#if defined (_WIN64) || (_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	#include "Core/Utility.hpp"
	//#include <dwmapi.h>
#endif

#include "Timer.hpp"


#if defined (_WIN64) || (_WINDOWS)
	#define PLATFORM_WIN64 1
#else
	#define PLATFORM_WIN64 0
#endif

#if WINVER < _WIN32_WINNT_WIN10	
	#error "Windows version below 10 is not supported!"
#endif

namespace lde
{

} // namespace lde
