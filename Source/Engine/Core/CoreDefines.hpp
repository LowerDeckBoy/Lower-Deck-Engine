#pragma once

/*
	Core/CoreDefines.hpp
	Definitions shared across Engine library content.
*/

#define DEBUG_MODE (_DEBUG) || (DEBUG)

#define VERSION_MAJOR "0"
#define VERSION_MINOR "0"
#define VERSION_PATCH "1"

#define ENGINE_VERSION std::format("{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)

#define RAYTRACING		0
#define MESH_SHADING	0

namespace lde
{

}
