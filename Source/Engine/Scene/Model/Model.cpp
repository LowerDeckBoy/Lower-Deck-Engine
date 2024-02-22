#include "RHI/D3D12/D3D12Context.hpp"
#include "Model.hpp"

#include "../Components/Components.hpp"
#include "Managers/AssetManager.hpp"
#include "Utility/FileSystem.hpp"

namespace lde
{
	Model::Model(RHI::D3D12Context* pGfx, std::string_view Filepath, World* pWorld)
	{
		ModelMesh = std::make_unique<Mesh>();
	
		auto& importer{ AssetManager::GetInstance() };
		importer.Import(pGfx, Filepath, pWorld, ModelMesh.get(), Vertices, Indices);
	
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
				Vertices.data(),
				static_cast<uint32>(Vertices.size()),
				Vertices.size() * sizeof(Vertices.at(0)),
				static_cast<uint32>(sizeof(Vertices.at(0)))
			},
			true // has SRV
			);
		
		IndexBuffer->Create(
			pGfx,
			RHI::BufferDesc{
				RHI::BufferUsage::eIndex,
				Indices.data(),
				static_cast<uint32>(Indices.size()),
				Indices.size() * sizeof(Indices.at(0)),
				static_cast<uint32>(sizeof(Indices.at(0)))
			}
		);
		

		ConstBuffer = new RHI::D3D12ConstantBuffer(&cbData, sizeof(cbData));
	}

} // namespace lde
