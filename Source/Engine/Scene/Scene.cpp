#include "Components/Components.hpp"
#include "Graphics/AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Scene.hpp"

namespace lde
{
	Scene::Scene(uint32 Width, uint32 Height, RHI::D3D12RHI* pGfx)
	{
		Initialize(Width, Height, pGfx);
	}

	Scene::~Scene()
	{
		for (auto& model : m_Models)
			model.reset();
	}
	
	void Scene::Initialize(uint32 Width, uint32 Height, RHI::D3D12RHI* pGfx)
	{
		m_World = new lde::World();
		m_Camera = std::make_unique<SceneCamera>(m_World, static_cast<float>((float)Width / (float)Height));
		m_Camera->InitializeInputs();
		m_Gfx = pGfx;
	
		Lights.emplace_back(Entity(m_World));
		//Lights.at(0).AddComponent<DirectLightComponent>();
	
		
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
		auto* commandList = m_Gfx->Device->GetGfxCommandList();
		commandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		struct vertex
		{
			uint32 index;
			uint32 offset;
		} vert{ pModel.VertexBuffer->GetSRVIndex(), 0 };

		for (uint32 i = 0; i < pModel.GetMesh()->Submeshes.size(); i++)
		{
			auto& mesh = pModel.GetMesh()->Submeshes.at(i);
			auto& transform = pModel.GetComponent<TransformComponent>();
			auto WVP = mesh.Matrix * transform.WorldMatrix * m_Camera->GetViewProjection();
			
			RHI::cbPerObject update = { XMMatrixTranspose(WVP), DirectX::XMMatrixTranspose(transform.WorldMatrix) };
			pModel.ConstBuffer->Update(&update);
			m_Gfx->BindConstantBuffer(pModel.ConstBuffer, 0);
	
			vert.offset = mesh.BaseVertex;
			commandList->PushConstants(1, 2, &vert, 0);
			// Push Material as constants; 64bytes
			commandList->PushConstants(2, 16, &mesh.Mat, 0);
	
			if (pModel.IndexBuffer->GetDesc().Count != 0)
			{
				m_Gfx->BindIndexBuffer(pModel.IndexBuffer);
				m_Gfx->DrawIndexed(mesh.IndexCount, mesh.BaseIndex, mesh.BaseVertex);
			}
			else /* Draw non-indexed */
			{
				m_Gfx->Draw(mesh.VertexCount);
			}
		}
	}
	
	void Scene::AddModel(std::string_view Filepath)
	{
		m_Models.emplace_back(std::make_unique<Model>(m_Gfx, Filepath, m_World));
	}

} // namespace lde
