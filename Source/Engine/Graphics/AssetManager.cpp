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
		
		ProcessNode(scene, &pInMesh, scene->mRootNode, nullptr, XMMatrixIdentity());
		pInMesh.Vertices = m_Vertices;
		pInMesh.Indices = m_Indices;
		timer.End(Files::GetFileName(Filepath));
	
		m_Vertices.clear();
		m_Vertices.shrink_to_fit();
		m_Indices.clear();
		m_Indices.shrink_to_fit();
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
	
		if (pNode->mMeshes)
		{	
			const auto aabb = pScene->mMeshes[0]->mAABB;

			pInMesh->AABB.Min.x = static_cast<float>(aabb.mMin.x);
			pInMesh->AABB.Min.y = static_cast<float>(aabb.mMin.y);
			pInMesh->AABB.Min.z = static_cast<float>(aabb.mMin.z);
			pInMesh->AABB.Max.x = static_cast<float>(aabb.mMax.x);
			pInMesh->AABB.Max.y = static_cast<float>(aabb.mMax.y);
			pInMesh->AABB.Max.z = static_cast<float>(aabb.mMax.z);

			for (uint32_t i = 0; i < pNode->mNumMeshes; i++)
			{
				pInMesh->Submeshes.emplace_back(ProcessMesh(pScene, pScene->mMeshes[pNode->mMeshes[i]], next));
			}
		}
	}

	Submesh AssetManager::ProcessMesh(const aiScene* pScene, const aiMesh* pMesh, XMMATRIX Matrix)
	{
		Submesh newSubmesh;

		ProcessGeometry(newSubmesh, pMesh);
		ProcessMaterials(pScene, newSubmesh, pMesh);
	
		newSubmesh.Matrix = Matrix;

		return newSubmesh;
	}

	void AssetManager::ProcessGeometry(Submesh& Submesh, const aiMesh* pMesh)
	{
		Submesh.BaseIndex	= static_cast<uint32>(m_Indices.size());
		Submesh.BaseVertex	= static_cast<uint32>(m_Vertices.size());
		Submesh.VertexCount = static_cast<uint32_t>(pMesh->mNumVertices);

		m_Vertices.reserve(m_Vertices.size() + pMesh->mNumVertices);
		for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex vertex{};

			if (pMesh->HasPositions())
			{
				vertex.Position = XMFLOAT3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
			}

			if (pMesh->mTextureCoords[0])
			{
				vertex.TexCoord = XMFLOAT2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
			}

			if (pMesh->HasNormals())
			{
				vertex.Normal = XMFLOAT3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
			}

			if (pMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent   = XMFLOAT3(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
				vertex.Bitangent = XMFLOAT3(pMesh->mBitangents[i].x, pMesh->mBitangents[i].y, pMesh->mBitangents[i].z);
			}

			m_Vertices.push_back(vertex);
		}

		uint32_t indexCount = 0;
		if (pMesh->HasFaces())
		{
			for (uint32_t i = 0; i < pMesh->mNumFaces; i++)
			{
				aiFace& face = pMesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++)
				{
					m_Indices.push_back(face.mIndices[j]);
					indexCount++;
				}
			}
		}
		Submesh.IndexCount = static_cast<uint32>(indexCount);
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

			aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &newMaterial.MetallicFactor);
			aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &newMaterial.RoughnessFactor);
		}
		
		if (material->GetTexture(aiTextureType_EMISSIVE, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.EmissiveIndex = textureManager.Create(m_Gfx, texPath);

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &colorFactor);
			newMaterial.EmissiveFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
		}
		
		aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF, &newMaterial.AlphaCutoff);
	
		Submesh.Mat = newMaterial;
	}

} // namespace lde
