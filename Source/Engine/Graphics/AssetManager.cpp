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

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

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
			aiProcess_GenBoundingBoxes |
			aiProcess_ImproveCacheLocality;
			//aiProcess_OptimizeGraph |
			//aiProcess_OptimizeMeshes |
	
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Filepath.data(), (uint32)LoadFlags);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			::MessageBoxA(nullptr, importer.GetErrorString(), "Import Error", MB_OK);
			throw std::runtime_error(importer.GetErrorString());
		}
	
		m_Filepath = Filepath.data();
		
		//ProcessNode(scene, &pInMesh, scene->mRootNode, nullptr, XMMatrixIdentity());

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

	}

	void AssetManager::ImportGLTF(RHI::D3D12RHI* /* pGfx */, std::string_view Filepath, Mesh& pInMesh)
	{
		Utility::LoadTimer timer;
		timer.Start();

		cgltf_options options{};
		cgltf_data* data = nullptr;

		cgltf_result result = cgltf_parse_file(&options, Filepath.data(), &data);
		if (result != cgltf_result_success)
		{
			LOG_ERROR("Failed to load via cgltf!");
		}

		if (cgltf_load_buffers(&options, data, Filepath.data()) != cgltf_result_success)
		{
			LOG_ERROR("Failed to load buffers via cgltf!");
		}

		if (cgltf_validate(data) != cgltf_result_success)
		{
			LOG_ERROR("Failed to validate cgltf!");
		}

		//std::vector<Vertex> verts;
		//std::vector<uint32> indices;
		
		for (uint32 meshIdx = 0; meshIdx < data->meshes_count; ++meshIdx)
		{
			auto mesh = &data->meshes[meshIdx];

			Submesh submesh{};

			//ProcessGeometry(submesh, mesh, verts, indices);
			ProcessGeometry(mesh, pInMesh.Submeshes, pInMesh.Vertices, pInMesh.Indices);

			pInMesh.Submeshes.push_back(submesh);
		}

		OptimizeMesh(pInMesh.Vertices, pInMesh.Indices);

		cgltf_free(data);

		timer.End("cgltf load time: ");
	}

	void AssetManager::OptimizeMesh(std::vector<Vertex>& Vertices, std::vector<uint32>& Indices)
	{
		std::vector<Vertex> tempVertices;
		std::vector<uint32> tempIndices;

		std::vector<uint32> remap(Indices.size());
		size_t remapSize = meshopt_generateVertexRemap(remap.data(),
			Indices.data(), Indices.size(), 
			Vertices.data(), Vertices.size(),
			sizeof(Vertex));

		tempIndices.resize(Indices.size());
		tempVertices.resize(remapSize);

		meshopt_remapVertexBuffer(tempVertices.data(), Vertices.data(), Vertices.size(), sizeof(Vertex), remap.data());
		meshopt_remapIndexBuffer(tempIndices.data(), Indices.data(), Indices.size(), remap.data());
		//meshopt_optimizeVertexCache(tempIndices.data(), tempIndices.data(), tempIndices.size(), tempVertices.size());
		//meshopt_optimizeOverdraw(tempIndices.data(), tempIndices.data(), tempIndices.size(), &(tempVertices[0].Position.x), tempVertices.size(), sizeof(Vertex), 1.05f);
		//meshopt_optimizeVertexFetch(tempVertices.data(), tempIndices.data(), tempIndices.size(), tempVertices.data(), remapSize, sizeof(Vertex));

		//Vertices.insert(Vertices.end(), tempVertices.begin(), tempVertices.end());
		//Indices.insert(Indices.end(), tempIndices.begin(), tempIndices.end());

		Vertices = tempVertices;
		Indices = tempIndices;

	}

	void AssetManager::ProcessNode(const aiScene* pScene, Mesh* pInMesh, const aiNode* pNode, Node* ParentNode, DirectX::XMMATRIX ParentMatrix)
	{
		Node* newNode = new Node();
		newNode->Parent = ParentNode;
		newNode->Name	= std::string(pNode->mName.C_Str());
		newNode->Matrix = ParentMatrix;

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
	

		if (!ParentNode)
		{
			// push_back nodes
		}
		else
		{
			// push_back children
		}

	}

	void AssetManager::ProcessGeometry(Submesh& Submesh, const aiMesh* pMesh, std::vector<Vertex>& OutVertices, std::vector<uint32>& OutIndices)
	{
		Submesh.BaseIndex	= static_cast<uint32>(OutIndices.size());
		Submesh.BaseVertex	= static_cast<uint32>(OutVertices.size());
		Submesh.VertexCount = static_cast<uint32>(pMesh->mNumVertices);
		
		Submesh.AABB.Min = *(XMFLOAT3*)&pMesh->mAABB.mMin;
		Submesh.AABB.Max = *(XMFLOAT3*)&pMesh->mAABB.mMax;

		OutVertices.reserve(OutVertices.size() + pMesh->mNumVertices);
		for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
		{
			Vertex vertex{};

			if (pMesh->HasPositions())
			{
				vertex.Position = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mVertices[i]));
				// If right-handed
				//vertex.Position = XMFLOAT3(pMesh->mVertices[i].z, pMesh->mVertices[i].y, pMesh->mVertices[i].x);
			}

			if (pMesh->mTextureCoords[0])
			{
				vertex.TexCoord = *(XMFLOAT2*)(reinterpret_cast<XMFLOAT2*>(&pMesh->mTextureCoords[0][i]));
			}

			if (pMesh->HasNormals())
			{
				vertex.Normal = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mNormals[i]));
			}

			if (pMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mTangents[i]));
				vertex.Bitangent = *(XMFLOAT3*)(reinterpret_cast<XMFLOAT3*>(&pMesh->mBitangents[i]));
			}

			OutVertices.push_back(vertex);
		}

		OutIndices.reserve(OutIndices.size() + (size_t)(pMesh->mNumFaces * 3));
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
			Submesh.Material = newMaterial;
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
		
		Submesh.Material = newMaterial;
	}

	void AssetManager::ProcessGeometry(const cgltf_mesh* pMesh, std::vector<Submesh>& Submeshes, std::vector<Vertex>& OutVertices, std::vector<uint32>& OutIndices)
	{
		for (uint32 prim = 0; prim < pMesh->primitives_count; ++prim)
		{
			auto primitive = pMesh->primitives[prim];

			Submesh submesh{};

			submesh.BaseIndex	= static_cast<uint32>(OutIndices.size());
			submesh.BaseVertex	= static_cast<uint32>(OutVertices.size());
			
			// Indices
			{
				auto accessor = primitive.indices;
				auto bufferView = accessor->buffer_view;
				auto buffer = bufferView->buffer;

				uint8_t* data = (uint8_t*)buffer->data + accessor->offset + bufferView->offset;

				submesh.IndexCount = static_cast<uint32>(accessor->count);

				if (accessor->stride == 1)
				{
					for (uint32 i = 0; i < accessor->count; i += 3)
					{
						OutIndices.push_back(((uint8_t*)data)[i + 0]);
						OutIndices.push_back(((uint8_t*)data)[i + 1]);
						OutIndices.push_back(((uint8_t*)data)[i + 2]);
					}
				}
				else if (accessor->stride == 2)
				{
					for (uint32 i = 0; i < accessor->count; i += 3)
					{
						OutIndices.push_back(((uint16_t*)data)[i + 0]);
						OutIndices.push_back(((uint16_t*)data)[i + 1]);
						OutIndices.push_back(((uint16_t*)data)[i + 2]);
					}
				}
				else if (accessor->stride == 4)
				{
					for (uint32 i = 0; i < accessor->count; i += 3)
					{
						OutIndices.push_back(((uint32_t*)data)[i + 0]);
						OutIndices.push_back(((uint32_t*)data)[i + 1]);
						OutIndices.push_back(((uint32_t*)data)[i + 2]);
					}
				}

				Submeshes.push_back(submesh);
			}

			std::vector<DirectX::XMFLOAT3> positions;
			std::vector<DirectX::XMFLOAT2> texcoords;
			std::vector<DirectX::XMFLOAT3> normals;
			std::vector<DirectX::XMFLOAT3> tangents;
			std::vector<float>			   tangents_ws;

			// Vertices
			for (uint32 attrib = 0; attrib < primitive.attributes_count; ++attrib)
			{
				auto attribute = &primitive.attributes[attrib];

				auto accessor = attribute->data;
				auto bufferView = accessor->buffer_view;
				auto buffer = bufferView->buffer;

				uint8_t* data = (uint8_t*)buffer->data + bufferView->offset + accessor->offset;

				submesh.VertexCount = static_cast<uint32>(accessor->count);
				
				switch (attribute->type)
				{
				case cgltf_attribute_type_position:
				{
					for (usize i = 0; i < accessor->count; ++i)
					{	
						XMFLOAT3 pos = *(DirectX::XMFLOAT3*)(data + i * accessor->stride);
						//positions.emplace_back(pos.z, pos.y, pos.x);
						positions.emplace_back(pos.x, pos.y, pos.z);

					}
					break;
				}
				case cgltf_attribute_type_texcoord:
				{
					for (usize i = 0; i < accessor->count; ++i)
					{
						texcoords.push_back(*(DirectX::XMFLOAT2*)(data + i * accessor->stride));

					}
					break;
				}
				case cgltf_attribute_type_normal:
				{
					for (usize i = 0; i < accessor->count; ++i)
					{
						normals.push_back(*(DirectX::XMFLOAT3*)(data + i * accessor->stride));

					}
					break;
				}
				case cgltf_attribute_type_tangent:
				{
					for (usize i = 0; i < accessor->count; ++i)
					{
						//XMFLOAT4 tangent 
						//tangents.push_back(*(DirectX::XMFLOAT3*)(data + i * accessor->stride));

						DirectX::XMFLOAT4 tangent = *(DirectX::XMFLOAT4*)((size_t)data + i * accessor->stride);
						tangents.push_back({ tangent.x, tangent.y, tangent.z });

						tangents_ws.push_back(tangent.w);
					}
					break;
				}
				}
			}

			if (texcoords.size() != positions.size())
				texcoords.resize(positions.size());
			if (normals.size() != positions.size())
				normals.resize(positions.size());
			if (tangents.size() != positions.size())
				tangents.resize(positions.size());
			if (tangents_ws.size() != positions.size())
				tangents_ws.resize(positions.size());

			for (usize i = 0; i < positions.size(); ++i)
			{
				XMFLOAT3 bitangent;
				XMVECTOR _bitangent = XMVectorScale(XMVector3Cross(XMLoadFloat3(&normals.at(i)), XMLoadFloat3(&tangents.at(i))), tangents_ws.at(i));
				XMStoreFloat3(&bitangent, XMVector3Normalize(_bitangent));

				Vertex vertex{
					.Position = positions.at(i),
					.TexCoord = texcoords.at(i),
					.Normal = normals.at(i),
					.Tangent = tangents.at(i),
					.Bitangent = bitangent
				};
				OutVertices.push_back(vertex);
				//Submesh.VertexCount = positions.size();
			}
		}

	}

} // namespace lde
