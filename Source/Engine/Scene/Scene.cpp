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
		{
			model.reset();
		}
	}
	
	void Scene::Initialize(uint32 Width, uint32 Height, RHI::D3D12RHI* pGfx)
	{
		m_World = new lde::World();
		m_Camera = std::make_unique<SceneCamera>(m_World, static_cast<float>((float)Width / (float)Height));
		m_Camera->InitializeInputs();
		m_Gfx = pGfx;
		
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
		
		auto* indexBuffer  = m_Gfx->Device->Buffers.at(pModel.GetMesh()->IndexBuffer);
		auto* vertexBuffer = m_Gfx->Device->Buffers.at(pModel.GetMesh()->VertexBuffer);
		auto* constBuffer  = m_Gfx->Device->ConstantBuffers.at(pModel.GetMesh()->ConstBuffer);

		auto& transform = pModel.GetComponent<TransformComponent>();

		struct meshVertex
		{
			uint32 index;
			uint32 offset;
		} vertex{ .index = vertexBuffer->GetSRVIndex(), .offset = 0 };

		for (uint32 i = 0; i < pModel.GetMesh()->Submeshes.size(); i++)
		{
			auto& mesh = pModel.GetMesh()->Submeshes.at(i);

			auto WVP = mesh.Matrix * transform.WorldMatrix * m_Camera->GetViewProjection();
			
			RHI::cbPerObject update = { XMMatrixTranspose(WVP), DirectX::XMMatrixTranspose(transform.WorldMatrix) };
			constBuffer->Update(&update);
			m_Gfx->BindConstantBuffer(constBuffer, 0);

			vertex.offset = mesh.BaseVertex;
			commandList->PushConstants(1, 2, &vertex);
			// Push Material as constants; 64 bytes
			commandList->PushConstants(2, 16, &mesh.Mat, 0);
			//commandList->Get()->ExecuteIndirect()
			if (indexBuffer->GetDesc().Count != 0)
			{
				//m_Gfx->BindIndexBuffer(indexBuffer);
				m_Gfx->BindIndexBuffer(pModel.GetMesh()->IndexView);
				m_Gfx->DrawIndexed(mesh.IndexCount, mesh.BaseIndex, mesh.BaseVertex);
			}
			else // Draw non-indexed
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
