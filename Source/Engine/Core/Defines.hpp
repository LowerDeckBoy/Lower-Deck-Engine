#pragma once

/*=============================================================
	Core/Defines.hpp
	Definitions shared across Engine library content.
=============================================================*/

#define DEBUG_MODE (_DEBUG) || (DEBUG)

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define STRINGIFY(Arg) #Arg
#define STR(Arg) STRINGIFY(Arg)

#define ENGINE_VERSION \
	STR(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)

#define RAYTRACING		0
#define MESH_SHADING	0

