#include "AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Scene/Model/Model.hpp"
#include "TextureManager.hpp"
#include <Core/Logger.hpp>
#include <Utility/FileSystem.hpp>
#include <Utility/Utility.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <meshoptimizer/meshoptimizer.h>

namespace lde
{
	AssetManager* AssetManager::m_Instance = nullptr;

	AssetManager::AssetManager()
	{
		m_Instance = this;
		LOG_INFO("AssetManager initialized.");
	}

	AssetManager::~AssetManager()
	{	
		LOG_INFO("AssetManager released.");
	}

	AssetManager& AssetManager::GetInstance()
	{
		if (!m_Instance)
		{
			m_Instance = new AssetManager();
			LOG_DEBUG("AssetManager instance recreated!.");
		}
		return *m_Instance;
	}

	void AssetManager::Import(RHI::D3D12RHI* pGfx, std::string_view Filepath, Mesh& pInMesh)
	{
		m_Gfx = pGfx;
		Utility::LoadTimer timer;
		timer.Start();

		constexpr int32 LoadFlags = 
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_JoinIdenticalVertices  |
			aiProcess_PreTransformVertices |
			aiProcess_GenBoundingBoxes;
	
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Filepath.data(), (uint32)LoadFlags);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			::MessageBoxA(nullptr, importer.GetErrorString(), "Import Error", MB_OK);
			throw std::runtime_error(importer.GetErrorString());
		}
	
		m_Filepath = Filepath.data();
		
		//ProcessNode(scene, &pInMesh, scene->mRootNode, nullptr, XMMatrixIdentity());
		
		const auto aabb = scene->mMeshes[0]->mAABB;

		pInMesh.AABB.Min.x = static_cast<float>(aabb.mMin.x);
		pInMesh.AABB.Min.y = static_cast<float>(aabb.mMin.y);
		pInMesh.AABB.Min.z = static_cast<float>(aabb.mMin.z);
		pInMesh.AABB.Max.x = static_cast<float>(aabb.mMax.x);
		pInMesh.AABB.Max.y = static_cast<float>(aabb.mMax.y);
		pInMesh.AABB.Max.z = static_cast<float>(aabb.mMax.z);

		if (scene->mName.length != 0)
		{
			pInMesh.Name = scene->mName.C_Str();
		}
		else
		{
			pInMesh.Name = Files::GetFileName(Filepath).c_str();
		}

		pInMesh.Submeshes.reserve(scene->mNumMeshes);
		for (uint32 meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx)
		{
			const auto mesh = scene->mMeshes[meshIdx];

			Submesh submesh;
			
			ProcessGeometry(submesh, mesh, pInMesh.Vertices, pInMesh.Indices);
			ProcessMaterials(scene, submesh, mesh);

			pInMesh.Submeshes.emplace_back(submesh);	
		}

		timer.End(Files::GetFileName(Filepath));

		//std::vector<meshopt_Meshlet> meshlets;
		//meshopt_buildMeshlets(meshlets.data(), )
	}

	void AssetManager::ProcessNode(const aiScene* pScene, Mesh* pInMesh, const aiNode* pNode, Node* ParentNode, DirectX::XMMATRIX ParentMatrix)
	{
		Node* newNode = new Node();
		newNode->Parent = ParentNode;
		newNode->Name = std::string(pNode->mName.C_Str());
	
		const auto transform = [&]() {
			if (!pNode->mTransformation.IsIdentity())
			{
				XMFLOAT4X4 temp = XMFLOAT4X4();
				temp._11 = static_cast<float>(pNode->mTransformation.a1);
				temp._12 = static_cast<float>(pNode->mTransformation.a2);
				temp._13 = static_cast<float>(pNode->mTransformation.a3);
				temp._14 = static_cast<float>(pNode->mTransformation.a4);
				temp._21 = static_cast<float>(pNode->mTransformation.b1);
				temp._22 = static_cast<float>(pNode->mTransformation.b2);
				temp._23 = static_cast<float>(pNode->mTransformation.b3);
				temp._24 = static_cast<float>(pNode->mTransformation.b4);
				temp._31 = static_cast<float>(pNode->mTransformation.c1);
				temp._32 = static_cast<float>(pNode->mTransformation.c2);
				temp._33 = static_cast<float>(pNode->mTransformation.c3);
				temp._34 = static_cast<float>(pNode->mTransformation.c4);
				temp._41 = static_cast<float>(pNode->mTransformation.d1);
				temp._42 = static_cast<float>(pNode->mTransformation.d2);
				temp._43 = static_cast<float>(pNode->mTransformation.d3);
				temp._44 = static_cast<float>(pNode->mTransformation.d4);
				newNode->Matrix = XMLoadFloat4x4(&temp);
			}
			else
			{
				aiVector3D		translation;
				aiQuaternion	rotation;
				aiVector3D		scale;
	
				pNode->mTransformation.Decompose(scale, rotation, translation);
				newNode->Translation	= XMFLOAT3(translation.x, translation.y, translation.z);
				newNode->Rotation		= XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				newNode->Scale			= XMFLOAT3(scale.x, scale.y, scale.z);
			}
			};
		transform();
	
		XMMATRIX local = newNode->Matrix * XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation));
		XMMATRIX next = local * ParentMatrix;
	
		if (pNode->mChildren)
		{
			for (size_t i = 0; i < pNode->mNumChildren; i++)
			{
				ProcessNode(pScene, pInMesh, pNode->mChildren[i], newNode, next);
			}
		}
	
	}

	void AssetManager::ProcessGeometry(Submesh& Submesh, const aiMesh* pMesh, std::vector<Vertex>& OutVertices, std::vector<uint32>& OutIndices)
	{
		Submesh.BaseIndex = static_cast<uint32>(OutIndices.size());
		Submesh.BaseVertex = static_cast<uint32>(OutVertices.size());
		Submesh.VertexCount = static_cast<uint32>(pMesh->mNumVertices);

		OutVertices.reserve(OutVertices.size() + pMesh->mNumVertices);
		for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
		{
			Vertex vertex{};

			if (pMesh->HasPositions())
			{
				vertex.Position = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mVertices[i]));
			}

			if (pMesh->mTextureCoords[0])
			{
				vertex.TexCoord = *(XMFLOAT2*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mTextureCoords[0][i]));
			}

			if (pMesh->HasNormals())
			{
				vertex.Normal = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mNormals[i]));;
			}

			if (pMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mTangents[i]));
				vertex.Bitangent = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mBitangents[i]));
			}

			OutVertices.push_back(vertex);
		}

		if (pMesh->HasFaces())
		{
			for (uint32_t i = 0; i < pMesh->mNumFaces; ++i)
			{
				aiFace& face = pMesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; ++j)
				{
					OutIndices.push_back(face.mIndices[j]);
					Submesh.IndexCount++;
				}
			}
		}
	}

	void AssetManager::ProcessMaterials(const aiScene* pScene, Submesh& Submesh, const aiMesh* pMesh) const
	{
		Material newMaterial{};
	
		if (pMesh->mMaterialIndex < 0)
		{
			Submesh.Mat = newMaterial;
			return;
		}
	
		auto& textureManager = TextureManager::GetInstance();
	
		aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];
	
		aiString materialPath{};
		if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.BaseColorIndex = textureManager.Create(m_Gfx, texPath);

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &colorFactor);
			newMaterial.BaseColorFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
		}
		
		if (material->GetTexture(aiTextureType_NORMALS, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.NormalIndex = textureManager.Create(m_Gfx, texPath);
		}

		if (material->GetTexture(aiTextureType_METALNESS, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.MetalRoughnessIndex = textureManager.Create(m_Gfx, texPath);
		}
			
		if (material->GetTexture(aiTextureType_EMISSIVE, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.EmissiveIndex = textureManager.Create(m_Gfx, texPath);

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &colorFactor);
			newMaterial.EmissiveFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
		}
		
		aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR,  &newMaterial.MetallicFactor);
		aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &newMaterial.RoughnessFactor);
		aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF, &newMaterial.AlphaCutoff);
		
		Submesh.Mat = newMaterial;
	}

} // namespace lde
