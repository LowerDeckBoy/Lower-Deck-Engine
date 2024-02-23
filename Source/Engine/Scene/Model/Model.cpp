#include "../Components/Components.hpp"
#include "Managers/AssetManager.hpp"
#include "Model.hpp"
#include "RHI/D3D12/D3D12Context.hpp"

namespace lde
{
	Model::Model(RHI::RHI* pRHI, std::string_view Filepath, World* pWorld)
	{
		m_Mesh = std::make_unique<Mesh>();
	
		auto& importer{ AssetManager::GetInstance() };
		//importer.Import((D3D12Context*)pGfx, Filepath, pWorld, ModelMesh.get(), Vertices, Indices);
		auto pGfx = (RHI::D3D12Context*)pRHI;
		importer.Import(pGfx, Filepath, *m_Mesh.get());
	
		VertexBuffer = new RHI::D3D12Buffer();
		IndexBuffer = new RHI::D3D12Buffer();

		Create(pGfx, pWorld);
		
	}

	Model::~Model()
	{
		delete ConstBuffer;
		VertexBuffer->Release();
		IndexBuffer->Release();
	}

	void Model::Create(RHI::D3D12Context* pGfx, World* pWorld)
	{
		// Default components
		Entity::Create(pWorld);
		AddComponent<TagComponent>("Model");
		AddComponent<TransformComponent>();
		
		VertexBuffer->Create(
			pGfx,
			RHI::BufferDesc{
				RHI::BufferUsage::eStructured,
				m_Mesh->Vertices.data(),
				static_cast<uint32>(m_Mesh->Vertices.size()),
				m_Mesh->Vertices.size() * sizeof(m_Mesh->Vertices.at(0)),
				static_cast<uint32>(sizeof(m_Mesh->Vertices.at(0)))
			},
			true // has SRV
		);

		IndexBuffer->Create(
			pGfx,
			RHI::BufferDesc{
				RHI::BufferUsage::eIndex,
				m_Mesh->Indices.data(),
				static_cast<uint32>(m_Mesh->Indices.size()),
				m_Mesh->Indices.size() * sizeof(m_Mesh->Indices.at(0)),
				static_cast<uint32>(sizeof(m_Mesh->Indices.at(0)))
			}
		);
		

		ConstBuffer = new RHI::D3D12ConstantBuffer(&cbData, sizeof(cbData));
	}

	Mesh* Model::GetMesh()
	{
		return m_Mesh.get();
	}

} // namespace lde
