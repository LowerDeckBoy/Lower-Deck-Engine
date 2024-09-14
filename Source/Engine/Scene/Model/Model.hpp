#pragma once
#include "../Entity.hpp"
#include "Mesh.hpp"
#include "Meshlet.hpp"
#include "RHI/D3D12/D3D12Buffer.hpp"

namespace lde
{
	class D3D12RHI;
	class RHI;
	class Buffer;
	
	class Model : public Entity
	{
	public:
		Model() {}
		//Model(D3D12RHI* pGfx, std::string_view Filepath, World* pWorld);
		Model(RHI* pRHI, std::string_view Filepath, World* pWorld);
		~Model();
	
		void Create(D3D12RHI* pGfx, World* pWorld);
	
		Mesh* GetMesh();

		std::string Filepath;


#if MESH_SHADING
		std::vector<DirectX::Meshlet>	Meshlets;
		std::vector<uint32>				UniqueIndices;
		std::vector<uint8>				UniqueVertexIB;
		std::vector<MeshletTriangle>	Triangles;

		BufferHandle MeshletBuffer;
		BufferHandle UniqueVertexIBBuffer;
		BufferHandle TrianglesBuffer;

		//void Meshletize();

#endif

	private:
		std::unique_ptr<Mesh> m_Mesh;

		std::vector<Material> m_Materials;

	};
} // namespace lde
