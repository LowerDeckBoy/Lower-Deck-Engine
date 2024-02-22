#pragma once

/*
	Core/CoreDefines.hpp
	Definitions shared across Engine library content.
*/

namespace lde
{

#define DEBUG_MODE (_DEBUG) || (DEBUG)

#define VERSION_MAJOR "0"
#define VERSION_MINOR "0"
#define VERSION_PATCH "1"
//#define ENGINE_VERSION std::string(VERSION_MAJOR + "." + VERSION_MINOR + "." + VERSION_PATCH)
#define ENGINE_VERSION std::format("{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)

#if defined RHI_D3D12
	constexpr const char* BACKEND = "D3D12";
#else
	constexpr const char* BACKEND = "Vulkan";
#endif

}
