#include "../Components/Components.hpp"
#include "Graphics/AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Model.hpp"

namespace lde
{
	Model::Model(RHI* pRHI, std::string_view Filepath, World* pWorld)
	{
		m_Mesh = std::make_unique<Mesh>();
	
		auto pGfx = (D3D12RHI*)pRHI;
		// TODO: Test loading times.
		AssetManager::GetInstance().Import(pGfx, Filepath, *m_Mesh.get());
		//AssetManager::GetInstance().ImportGLTF(pGfx, Filepath, *m_Mesh.get());

		Create(pGfx, pWorld);

	
	}

	Model::~Model()
	{

	}

	void Model::Create(D3D12RHI* pGfx, World* pWorld)
	{
		m_Mesh->Create(pGfx->Device.get());

		// Default components
		Entity::Create(pWorld);
		Entity::AddComponent<TagComponent>(m_Mesh->Name);
		Entity::AddComponent<TransformComponent>();
		
		m_Mesh->IndexView = GetIndexView(pGfx->Device->Buffers.at(m_Mesh->IndexBuffer));

		m_Mesh->ConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&m_Mesh->cbData, sizeof(m_Mesh->cbData));
	}

} // namespace lde
