#pragma once


#include <Core/CoreTypes.hpp>
#include <vector>

namespace lde
{
	struct Meshlet
	{
		uint32 VertexCount;
		uint32 VertexOffset;
		uint32 PrimitiveCount;
		uint32 PrimitiveOffset;
	};

	template<typename T>
	struct InlineMeshlet
	{
		struct Triangle
		{
			uint32 i0 : 10;
			uint32 i1 : 10;
			uint32 i2 : 10;
			const uint32 spare : 2;
		};
		
		std::vector<T> Uniques;
		std::vector<Triangle> PrimitiveIndices;

	};

	void Meshletize();
}
