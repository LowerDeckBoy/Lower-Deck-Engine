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
		AssetManager::GetInstance().Import(pGfx, Filepath, *m_Mesh.get());

		Create(pGfx, pWorld);
		
	}

	Model::~Model()
	{

	}

	void Model::Create(RHI::D3D12RHI* pGfx, World* pWorld)
	{
		// Default components
		Entity::Create(pWorld);
		AddComponent<TagComponent>("Model");
		AddComponent<TransformComponent>();
		
		VertexBuffer = pGfx->GetDevice()->CreateBuffer(
			RHI::BufferDesc{
				RHI::BufferUsage::eStructured,
				m_Mesh->Vertices.data(),
				static_cast<uint32>(m_Mesh->Vertices.size()),
				m_Mesh->Vertices.size() * sizeof(m_Mesh->Vertices.at(0)),
				static_cast<uint32>(sizeof(m_Mesh->Vertices.at(0))),
				true
			});

		
		IndexBuffer = pGfx->GetDevice()->CreateBuffer(
			RHI::BufferDesc{
				RHI::BufferUsage::eIndex,
				m_Mesh->Indices.data(),
				static_cast<uint32>(m_Mesh->Indices.size()),
				m_Mesh->Indices.size() * sizeof(m_Mesh->Indices.at(0)),
				static_cast<uint32>(sizeof(m_Mesh->Indices.at(0)))
			});

		//IndexView = RHI::GetIndexView(IndexBuffer);

		ConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&cbData, sizeof(cbData));
		
	}

	Mesh* Model::GetMesh()
	{
		return m_Mesh.get();
	}

} // namespace lde
