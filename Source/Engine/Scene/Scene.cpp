#include "RHI/D3D12/D3D12Context.hpp"
#include "Scene.hpp"
#include <Core/Logger.hpp>
#include "Managers/AssetManager.hpp"
#include "Components/Components.hpp"


namespace mf
{
	Scene::Scene(uint32 Width, uint32 Height, RHI::D3D12Context* pGfx)
	{
		Initialize(Width, Height, pGfx);
	}

	Scene::~Scene()
	{
		for (auto& model : m_Models)
			model.reset();
	}
	
	void Scene::Initialize(uint32 Width, uint32 Height, RHI::D3D12Context* pGfx)
	{
		m_World = new mf::World();
		m_Camera = std::make_unique<SceneCamera>(m_World, static_cast<float>((float)Width / (float)Height));
		m_Camera->InitializeInputs();
		m_Gfx = pGfx;
	
		Lights.emplace_back(Entity(m_World));
		Lights.at(0).AddComponent<DirectLightComponent>();
	
	}

	void Scene::OnResize(float AspectRatio)
	{
		m_Camera->OnAspectRatioChange(AspectRatio);
	}
	
	void Scene::DrawScene()
	{
		if (m_Models.empty())
			return;
	
		for (auto& model : m_Models)
		{
			DrawModel(*model);
		}
	}

	void Scene::DrawModel(Model& pModel)
	{
		m_Gfx->GraphicsCommandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		for (uint32 i = 0; i < pModel.ModelMesh->SubMeshes.size(); i++)
		{
			auto& mesh = pModel.ModelMesh->SubMeshes.at(i);
			auto& transform = pModel.GetComponent<TransformComponent>();
			auto WVP = DirectX::XMMatrixTranspose(mesh.Matrix * transform.WorldMatrix * m_Camera->GetViewProjection());
			
			RHI::cbPerObject update = { WVP, DirectX::XMMatrixTranspose(transform.WorldMatrix) };
			pModel.ConstBuffer->Update(&update);
			m_Gfx->BindConstantBuffer(pModel.ConstBuffer, 0);
	
			struct vertex
			{
				uint32 index;
				uint32 offset;
			} vert{ pModel.VertexBuffer->Descriptor().Index(), mesh.BaseVertex };
			m_Gfx->GraphicsCommandList->Get()->SetGraphicsRoot32BitConstants(1, 2, &vert, 0);
	
			// Push Material as constants; 64bytes
			m_Gfx->GraphicsCommandList->Get()->SetGraphicsRoot32BitConstants(2, 16, &mesh.Mat, 0);
	
			if (pModel.IndexBuffer->GetDesc().Count != 0)
			{
				m_Gfx->BindIndexBuffer(pModel.IndexBuffer);
				m_Gfx->DrawIndexed(mesh.IndexCount, mesh.BaseIndex, mesh.BaseVertex);
			}
			else /* Draw non-indexed */
			{

			}
		}
	}
	
	void Scene::AddModel(std::string_view Filepath)
	{
		m_Models.emplace_back(std::make_unique<Model>(m_Gfx, Filepath, m_World));
	}

} // namespace mf
