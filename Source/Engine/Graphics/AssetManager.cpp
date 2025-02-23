#include "AssetManager.hpp"
#include "Core/FileSystem.hpp"
#include "Core/Logger.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Core/Utility.hpp"
#include "Scene/Model/Model.hpp"
#include "TextureManager.hpp"
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

	void AssetManager::Import(D3D12RHI* pGfx, std::string_view Filepath, std::vector<StaticMesh>& InStaticMeshes)
	{
		m_Gfx = pGfx;

		constexpr int32 LoadFlags =
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_JoinIdenticalVertices |
			aiProcess_OptimizeMeshes |
			aiProcess_PreTransformVertices |
			aiProcess_GenBoundingBoxes |
			aiProcess_ImproveCacheLocality;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Filepath.data(), (uint32)LoadFlags);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			::MessageBoxA(nullptr, importer.GetErrorString(), "Import Error", MB_OK);
			throw std::runtime_error(importer.GetErrorString());
		}

		m_Filepath = Filepath.data();

		LoadStaticMesh(scene, InStaticMeshes);

	}

	void AssetManager::LoadStaticMesh(const aiScene* pScene, std::vector<StaticMesh>& InStaticMeshes)
	{
		for (uint32_t i = 0; i < pScene->mNumMeshes; ++i)
		{
			const auto& mesh = pScene->mMeshes[i];

			StaticMesh meshData{};

			meshData.AABB.Min = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMin);
			meshData.AABB.Max = *(DirectX::XMFLOAT3*)(&mesh->mAABB.mMax);

			// For later Meshlet building.
			std::vector<DirectX::XMFLOAT3> positions;

			for (uint32_t vertexId = 0; vertexId < mesh->mNumVertices; ++vertexId)
			{
				Vertex vertex{};

				if (mesh->HasPositions())
				{
					vertex.Position = *(DirectX::XMFLOAT3*)(&mesh->mVertices[vertexId]);
					//positions.push_back(vertex.Position);
				}

				if (mesh->HasTextureCoords(0))
				{
					vertex.TexCoord = *(DirectX::XMFLOAT2*)(&mesh->mTextureCoords[0][vertexId]);
				}

				if (mesh->HasNormals())
				{
					vertex.Normal = *(DirectX::XMFLOAT3*)(&mesh->mNormals[vertexId]);
				}

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = *(DirectX::XMFLOAT3*)(&mesh->mTangents[vertexId]);
					vertex.Bitangent = *(DirectX::XMFLOAT3*)(&mesh->mBitangents[vertexId]);
				}

				meshData.Vertices.push_back(vertex);
			}

			if (mesh->HasFaces())
			{

				for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx)
				{
					aiFace& face = mesh->mFaces[faceIdx];

					for (uint32_t idx = 0; idx < face.mNumIndices; ++idx)
					{
						meshData.Indices.push_back(face.mIndices[idx]);
					}
				}
			}

			meshData.NumVertices = static_cast<uint32>(meshData.Vertices.size());
			meshData.NumIndices	 = static_cast<uint32>(meshData.Indices.size());

			LoadMaterial(pScene, mesh, meshData);

			InStaticMeshes.push_back(meshData);
		}
	}

	void AssetManager::LoadMaterial(const aiScene* pScene, const aiMesh* pMesh, StaticMesh& InStaticMesh)
	{
		Material newMaterial{};

		if (pMesh->mMaterialIndex < 0)
		{
			InStaticMesh.Material = newMaterial;

			return;
		}

		auto& textureManager = TextureManager::GetInstance();

		aiMaterial* material = pScene->mMaterials[pMesh->mMaterialIndex];

		aiString materialPath{};
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &materialPath) == aiReturn_SUCCESS || material->GetTexture(aiTextureType_BASE_COLOR, 0, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath = Files::GetTexturePath(m_Filepath.data(), std::string(materialPath.C_Str()));

			newMaterial.BaseColorIndex = textureManager.Create(m_Gfx, texPath);

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &colorFactor);
			newMaterial.BaseColorFactor = *(DirectX::XMFLOAT4*)(&colorFactor);
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
			newMaterial.EmissiveFactor = *(DirectX::XMFLOAT4*)(&colorFactor);
		}

		aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &newMaterial.MetallicFactor);
		aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &newMaterial.RoughnessFactor);
		aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF, &newMaterial.AlphaCutoff);
		
		InStaticMesh.Material = newMaterial;
	}

	/*
	void AssetManager::ImportGLTF(D3D12RHI* , std::string_view Filepath, Mesh& pInMesh)
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
	
		for (uint32 meshIdx = 0; meshIdx < data->meshes_count; ++meshIdx)
		{
			auto mesh = &data->meshes[meshIdx];
	
			Submesh submesh{};
	
			//ProcessGeometry(submesh, mesh, verts, indices);
			ProcessGeometry(mesh, pInMesh.Submeshes, pInMesh.Vertices, pInMesh.Indices);
	
			pInMesh.Submeshes.push_back(submesh);
		}
	
		cgltf_free(data);
	
		timer.End("cgltf load time: ");
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
			}
		}

	}
	*/
} // namespace lde
