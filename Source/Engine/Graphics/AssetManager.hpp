#pragma once

//
//
//
//

#include <Core/CoreTypes.hpp>
#include <Core/String.hpp>
#include <vector>
#include <DirectXMath.h>

// assimp forwards
struct aiScene;
struct aiNode;
struct aiMesh;

namespace lde
{
	using namespace DirectX;
	namespace RHI { class D3D12RHI; }

	struct Vertex;
	struct Mesh;
	struct SubMesh;

	class Model;
	struct Node;
	class World;

	class AssetManager
	{
		static AssetManager* m_Instance;
	public:
		AssetManager();
		~AssetManager();
	
		static AssetManager& GetInstance();
	
		void Initialize(RHI::D3D12RHI* pGfx)
		{
			m_Gfx = pGfx;
		}
		void Import(RHI::D3D12RHI* pGfx, std::string_view Filepath, Mesh& pInMesh);
	
		// via cgltf
		//void ImportGLTF(std::string_view Filepath, Model* pModel);

	private:
		//void ImportGLTF(std::string_view Filepath, Model& Target);
		//void ImportFBX(const aiScene* pScene);
	
		void ProcessNode(const aiScene* pScene, Mesh* pInMesh, const aiNode* pNode, Node* ParentNode, DirectX::XMMATRIX ParentMatrix);
		SubMesh ProcessMesh(const aiScene* pScene, const aiMesh* pMesh, XMMATRIX Matrix);
		void ProcessVertices(SubMesh& Submesh, const aiMesh* pMesh);
		void ProcessIndices(SubMesh& Submesh, const aiMesh* pMesh);
	
		void ProcessMaterials(const aiScene* pScene, SubMesh& Submesh, const aiMesh* pMesh);
		
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32> m_Indices{};
		
		// For access to Device and CommandList
		RHI::D3D12RHI* m_Gfx = nullptr;
	
		std::string m_Filepath;
	
		aiScene* m_Scene = nullptr;

	};

} // namespace lde
