#include "RHI/D3D12/D3D12Context.hpp"
#include "Scene/Model/Model.hpp"

#include "AssetManager.hpp"
#include "TextureManager.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

#include "Utility/FileSystem.hpp"
#include "Utility/Utility.hpp"

namespace mf
{
	void AssetManager::Import(
			RHI::D3D12Context* pGfx, std::string_view Filepath,
			World* pWorld, Mesh* pInMesh, 
			std::vector<Vertex>& OutVertices, 
			std::vector<uint32>& OutIndices)
	{
		m_Gfx = pGfx;
		Utility::LoadTimer timer;
		timer.Start();
	
		constexpr auto LoadFlags = aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_JoinIdenticalVertices |
			aiProcess_PreTransformVertices;
	
		const aiScene* scene = m_Importer.ReadFile(Filepath.data(), LoadFlags);
	
		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			::MessageBoxA(nullptr, m_Importer.GetErrorString(), "Importer Error", MB_OK);
			throw std::runtime_error(m_Importer.GetErrorString());
		}
	
		m_Filepath = Filepath.data();
	
		ProcessNode(scene, pInMesh, scene->mRootNode, nullptr, XMMatrixIdentity());
		OutVertices = m_Vertices;
		OutIndices = m_Indices;
		timer.End(files::GetFileName(Filepath));
	
	}

	void AssetManager::ProcessNode(const aiScene* pScene, Mesh* pInMesh, const aiNode* pNode, Node* ParentNode, DirectX::XMMATRIX ParentMatrix)
	{
		Node* newNode = new Node();
		newNode->Parent = ParentNode;
		newNode->Name = std::string(pNode->mName.C_Str());
	
		const auto transform = [&]() {
			if (!pNode->mTransformation.IsIdentity())
			{
				XMFLOAT4X4 temp{ XMFLOAT4X4() };
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
				newNode->Translation = XMFLOAT3(translation.x, translation.y, translation.z);
				newNode->Rotation = XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				newNode->Scale = XMFLOAT3(scale.x, scale.y, scale.z);
			}
			};
		transform();
	
		XMMATRIX local{ XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation)) *XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation)) };
		XMMATRIX next{ local * ParentMatrix };
	
		if (pNode->mChildren)
		{
			for (size_t i = 0; i < pNode->mNumChildren; i++)
				ProcessNode(pScene, pInMesh, pNode->mChildren[i], newNode, next);
		}
	
		if (pNode->mMeshes)
		{
			for (uint32_t i = 0; i < pNode->mNumMeshes; i++)
			{
				pInMesh->SubMeshes.emplace_back(ProcessMesh(pScene, pScene->mMeshes[pNode->mMeshes[i]], next));
			}
		}
	}

	SubMesh AssetManager::ProcessMesh(const aiScene* pScene, const aiMesh* pMesh, XMMATRIX Matrix)
	{
		SubMesh newSubMesh;
		ProcessVertices(newSubMesh, pMesh);
		ProcessIndices(newSubMesh, pMesh);
		ProcessMaterials(pScene, newSubMesh, pMesh);
	
		newSubMesh.Matrix = Matrix;
		return newSubMesh;
	}

	void AssetManager::ProcessVertices(SubMesh& Submesh, const aiMesh* pMesh)
	{
		std::vector<XMFLOAT3> positions;
		std::vector<XMFLOAT2> uvs;
		std::vector<XMFLOAT3> normals;
		std::vector<XMFLOAT3> tangents;
		std::vector<XMFLOAT3> bitangents;
	
		Submesh.BaseIndex  = static_cast<uint32>(m_Indices.size());
		Submesh.BaseVertex = static_cast<uint32>(m_Vertices.size());
	
		for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
		{
			if (pMesh->HasPositions())
				positions.emplace_back(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
			else
				positions.emplace_back(0.0f, 0.0f, 0.0f);
	
			if (pMesh->mTextureCoords[0])
				uvs.emplace_back(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
			else
				uvs.emplace_back(0.0f, 0.0f);
	
			if (pMesh->HasNormals())
				normals.emplace_back(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
			else
				normals.emplace_back(0.0f, 0.0f, 0.0f);
	
			if (pMesh->HasTangentsAndBitangents())
			{
				tangents.emplace_back(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
				bitangents.emplace_back(pMesh->mBitangents[i].x, pMesh->mBitangents[i].y, pMesh->mBitangents[i].z);
			}
			else
			{
				tangents.emplace_back(0.0f, 0.0f, 0.0f);
				bitangents.emplace_back(0.0f, 0.0f, 0.0f);
			}
		}
	
		Submesh.VertexCount = static_cast<uint32_t>(positions.size());
		for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
		{
			//m_Vertices.emplace_back(positions.at(i), uvs.at(i), normals.at(i), tangents.at(i));
			m_Vertices.emplace_back(positions.at(i), uvs.at(i), normals.at(i), tangents.at(i), bitangents.at(i));
		}	
	}

	void AssetManager::ProcessIndices(SubMesh& Submesh, const aiMesh* pMesh)
	{
		uint32_t indexCount = 0;
		if (pMesh->HasFaces())
		{
			//newMesh->bHasIndices = true;
			for (uint32_t i = 0; i < pMesh->mNumFaces; i++)
			{
				aiFace& face{ pMesh->mFaces[i] };
				for (uint32_t j = 0; j < face.mNumIndices; j++)
				{
					m_Indices.push_back(face.mIndices[j]);
					indexCount++;
				}
			}
		}
		Submesh.IndexCount = static_cast<uint32>(indexCount);
	
	}

	void AssetManager::ProcessMaterials(const aiScene* pScene, SubMesh& Submesh, const aiMesh* pMesh)
	{
		Material newMaterial{};
	
		if (pMesh->mMaterialIndex < 0)
		{
			Submesh.Mat = newMaterial;
			return;
		}
	
		auto& textureManager{ TextureManager::GetInstance() };
	
		aiMaterial* material{ pScene->mMaterials[pMesh->mMaterialIndex] };
	
		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i)
		{
			aiString materialPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, i, &materialPath) == aiReturn_SUCCESS)
			{
				auto texPath{ files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str())) };
		
				newMaterial.BaseColorIndex = textureManager.Create(m_Gfx, texPath);
	
				aiColor4D colorFactor{};
				aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &colorFactor);
				newMaterial.BaseColorFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
			}
		}
	
		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); ++i)
		{
			aiString materialPath;
			if (material->GetTexture(aiTextureType_NORMALS, i, &materialPath) == aiReturn_SUCCESS)
			{
				auto texPath{ files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str())) };
	
				newMaterial.NormalIndex = textureManager.Create(m_Gfx, texPath);
			}
		}
	
		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_METALNESS); ++i)
		{
			aiString materialPath{};
			if (material->GetTexture(aiTextureType_METALNESS, i, &materialPath) == aiReturn_SUCCESS)
			{
				auto texPath{ files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str())) };
	
				newMaterial.MetalRoughnessIndex = textureManager.Create(m_Gfx, texPath);
	
				aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR,  &newMaterial.MetallicFactor);
				aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &newMaterial.RoughnessFactor);
			}
		}
	
		for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_EMISSIVE); ++i)
		{
			aiString materialPath{};
			if (material->GetTexture(aiTextureType_EMISSIVE, i, &materialPath) == aiReturn_SUCCESS)
			{
				auto texPath = files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));
	
				newMaterial.EmissiveIndex = textureManager.Create(m_Gfx, texPath);
	
				aiColor4D colorFactor{};
				aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &colorFactor);
				newMaterial.EmissiveFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
			}
		}
	
		aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF, &newMaterial.AlphaCutoff);
	
		Submesh.Mat = newMaterial;
	}

} // namespace mf
