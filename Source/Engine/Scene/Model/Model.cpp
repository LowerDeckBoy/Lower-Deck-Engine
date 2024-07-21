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

	#if MESH_SHADING

		std::vector<XMFLOAT3> pos(m_Mesh->Vertices.size());
		for (size_t j = 0; j < m_Mesh->Vertices.size(); ++j)
			pos[j] = m_Mesh->Vertices.at(j).Position;

		DirectX::ComputeMeshlets(
			m_Mesh->Indices.data(), m_Mesh->Indices.size() / 3,
			pos.data(), m_Mesh->Vertices.size(),
			UniqueIndices.data(), Meshlets, UniqueVertexIB, Triangles);
		//nullptr, Meshlets, uniqueVb, triangles);

	//const uint32* uniqueVertexIndices = reinterpret_cast<const uint32*>(uniqueVertexIB.data());
		size_t vertIndices = UniqueVertexIB.size() / sizeof(uint32);
		const char* msg = std::to_string(vertIndices).c_str() + '\n';
		::OutputDebugStringA(msg);

		MeshletBuffer = pGfx->Device->CreateBuffer(
			RHI::BufferDesc{
				RHI::BufferUsage::eStructured,
				Meshlets.data(),
				static_cast<uint32>(Meshlets.size()),
				Meshlets.size() * sizeof(Meshlets.at(0)),
				static_cast<uint32>(sizeof(Meshlets.at(0))),
				true
			});

		UniqueVertexIBBuffer = pGfx->Device->CreateBuffer(
			RHI::BufferDesc{
				RHI::BufferUsage::eStructured,
				UniqueVertexIB.data(),
				static_cast<uint32>(UniqueVertexIB.size()),
				UniqueVertexIB.size() * sizeof(UniqueVertexIB.at(0)),
				static_cast<uint32>(sizeof(UniqueVertexIB.at(0))),
				true
			});

		TrianglesBuffer = pGfx->Device->CreateBuffer(
			RHI::BufferDesc{
				RHI::BufferUsage::eStructured,
				Triangles.data(),
				static_cast<uint32>(Triangles.size()),
				Triangles.size() * sizeof(Triangles.at(0)),
				static_cast<uint32>(sizeof(Triangles.at(0))),
				true
			});

	#endif
	}

	Model::~Model()
	{

	}

	void Model::Create(RHI::D3D12RHI* pGfx, World* pWorld)
	{
		m_Mesh->Create(pGfx->Device.get());


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
