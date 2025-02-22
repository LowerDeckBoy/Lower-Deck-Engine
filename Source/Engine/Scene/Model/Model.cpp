#include "../Components/TransformComponent.hpp"
#include "../Components/NameComponent.hpp"
#include "Graphics/AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Model.hpp"

namespace lde
{
	void Model::Create(D3D12RHI* pGfx, World* pWorld)
	{
		// Default components
		Entity::Create(pWorld);
		Entity::AddComponent<TransformComponent>();
		
		ConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&cbData, sizeof(cbData));

		for (auto& mesh : StaticMeshes)
		{
			mesh.VertexBuffer = pGfx->Device->CreateBuffer(
				BufferDesc{
					BufferUsage::eStructured,
					mesh.Vertices.data(),
					mesh.NumVertices,
					mesh.NumVertices * sizeof(mesh.Vertices.at(0)),
					static_cast<uint32>(sizeof(mesh.Vertices.at(0))),
					true
				});

			mesh.IndexBuffer = pGfx->Device->CreateBuffer(
				BufferDesc{
					BufferUsage::eIndex,
					mesh.Indices.data(),
					mesh.NumIndices,
					mesh.NumIndices * sizeof(mesh.Indices.at(0)),
					static_cast<uint32>(sizeof(mesh.Indices.at(0)))
				});

			mesh.IndexBufferView = GetIndexView(pGfx->Device->Buffers.at(mesh.IndexBuffer));
		}

	}

} // namespace lde
