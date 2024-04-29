#pragma once


#include <Core/CoreTypes.hpp>
#include <vector>

namespace lde
{
	enum class Attribute : uint8
	{
		ePosition,
		eTexCoords,
		eNormal,
		eTangent,
		eBitangent
	};

	struct MeshletDesc
	{
		uint32 VertexCount;
		uint32 VertexOffset;
		uint32 PrimitiveCount;
		uint32 PrimitiveOffset;
	};

	void Meshletize();
}
