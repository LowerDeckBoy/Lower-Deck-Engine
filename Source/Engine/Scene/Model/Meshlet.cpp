#include "Mesh.hpp"
#include "Meshlet.hpp"

namespace lde
{
	void MeshletMesh::Meshletize()
	{
		std::vector<uint32> uniqueIndices;
		std::vector<uint8> uniqueVb;
		std::vector<DirectX::MeshletTriangle> triangles;

		std::vector<XMFLOAT3> pos(Vertices.size());
		for (size_t j = 0; j < Vertices.size(); ++j)
			pos[j] = Vertices.at(j).Position;
		//DirectX::ComputeMeshlets()
		DirectX::ComputeMeshlets(
			Indices.data(), Indices.size() / 3, 
			pos.data(), Vertices.size(),
			nullptr, Meshlets, uniqueVb, triangles);

	}
} // namespace lde
