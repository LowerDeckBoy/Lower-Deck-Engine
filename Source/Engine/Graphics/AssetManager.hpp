#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Core/CoreTypes.hpp>
#include <Core/String.hpp>
#include <DirectXMath.h>
#include <vector>

struct aiScene;
struct aiNode;
struct aiMesh;

struct cgltf_mesh;

namespace lde
{
	class D3D12RHI;

	struct Vertex;
	struct Mesh;
	struct Submesh;
	struct StaticMesh;

	class Model;
	class World;
	struct Node;

	class AssetManager
	{
		static AssetManager* m_Instance;
	public:
		AssetManager();
		~AssetManager();
	
		static AssetManager& GetInstance();
	
		void Initialize(D3D12RHI* pGfx)
		{
			m_Gfx = pGfx;
		}

		void Import(D3D12RHI* pGfx, std::string_view Filepath, std::vector<StaticMesh>& InStaticMeshes);

		void ImportGLTF(D3D12RHI* pGfx, std::string_view Filepath, Mesh& pInMesh);

		void LoadStaticMesh(const aiScene* pScene, std::vector<StaticMesh>& InStaticMeshes);
		void LoadMaterial(const aiScene* pScene, const aiMesh* pMesh, StaticMesh& InStaticMesh);

	private:
		[[maybe_unused]]
		void ProcessNode(const aiScene* pScene, Mesh* pInMesh, const aiNode* pNode, Node* ParentNode, DirectX::XMMATRIX ParentMatrix);

		//void ProcessGeometry(Submesh& Submesh, const cgltf_mesh* pMesh, std::vector<Vertex>& OutVertices, std::vector<uint32>& OutIndices);
		void ProcessGeometry(const cgltf_mesh* pMesh, std::vector<Submesh>& Submeshes, std::vector<Vertex>& OutVertices, std::vector<uint32>& OutIndices);

		// For access to Device and CommandList
		D3D12RHI* m_Gfx = nullptr;
	
		std::string m_Filepath;
	
		aiScene* m_Scene = nullptr;

	};

} // namespace lde
