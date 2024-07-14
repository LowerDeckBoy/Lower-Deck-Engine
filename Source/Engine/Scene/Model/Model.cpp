#include "../Components/Components.hpp"
#include "Graphics/AssetManager.hpp"
#include "Model.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"

namespace lde
{
	Model::Model(RHI::RHI* pRHI, std::string_view Filepath, World* pWorld)
	{
		m_Mesh = std::make_unique<Mesh>();
	
		auto pGfx = (RHI::D3D12RHI*)pRHI;
		// TODO: Test loading times.
		AssetManager::GetInstance().Import(pGfx, Filepath, *m_Mesh.get());
		//AssetManager::GetInstance().ImportGLTF(pGfx, Filepath, *m_Mesh.get());

		Create(pGfx, pWorld);
		
	}

	Model::~Model()
	{

	}

	void Model::Create(RHI::D3D12RHI* pGfx, World* pWorld)
	{
		m_Mesh->Create(pGfx->Device.get());

		/*
		
		std::vector<DirectX::Meshlet> Meshlets;
		std::vector<uint32> uniqueIndices;
		std::vector<uint8> uniqueVertexIB;
		std::vector<DirectX::MeshletTriangle> triangles;

		std::vector<XMFLOAT3> pos(m_Mesh->Vertices.size());
		for (size_t j = 0; j < m_Mesh->Vertices.size(); ++j)
			pos[j] = m_Mesh->Vertices.at(j).Position;
		
		DirectX::ComputeMeshlets(
			m_Mesh->Indices.data(), m_Mesh->Indices.size() / 3,
			pos.data(), m_Mesh->Vertices.size(),
			uniqueIndices.data(), Meshlets, uniqueVertexIB, triangles);
			//nullptr, Meshlets, uniqueVb, triangles);

		//const uint32* uniqueVertexIndices = reinterpret_cast<const uint32*>(uniqueVertexIB.data());
		size_t vertIndices = uniqueVertexIB.size() / sizeof(uint32);
		*/

		// Default components
		Entity::Create(pWorld);
		AddComponent<TagComponent>(m_Mesh->Name);
		AddComponent<TransformComponent>();
		
		m_Mesh->IndexView = RHI::GetIndexView(pGfx->Device->Buffers.at(m_Mesh->IndexBuffer));

		m_Mesh->ConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&m_Mesh->cbData, sizeof(m_Mesh->cbData));
	}

	Mesh* Model::GetMesh()
	{
		return m_Mesh.get();
	}

} // namespace lde
