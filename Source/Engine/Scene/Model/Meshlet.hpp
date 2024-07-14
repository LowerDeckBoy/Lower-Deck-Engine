#pragma once


#include <Core/CoreTypes.hpp>
#include <vector>

#include <DirectXMesh.h>

namespace lde
{
	static constexpr uint64 MESHLET_MAX_TRIANGLES	= 124;
	static constexpr uint64 MESHLET_MAX_VERTICES	= 64;

	//struct Meshlet
	//{
	//	uint32 VertexCount;
	//	uint32 VertexOffset;
	//	uint32 PrimitiveCount;
	//	uint32 PrimitiveOffset;
	//
	//	float Center[3];
	//	float Radius;
	//};

	// TODO:
	struct MeshletMesh
	{
		std::vector<DirectX::Meshlet>	Meshlets;
		std::vector<uint32>				Indices;
		std::vector<Vertex>				Vertices;

		void Meshletize();
	};
	
	//DirectX::ComputeMeshlets

}
