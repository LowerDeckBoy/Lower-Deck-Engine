#include "Components/TransformComponent.hpp"
#include "Components/NameComponent.hpp"
#include "Graphics/AssetManager.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Scene.hpp"
#include "Components/LightComponent.hpp"

namespace lde
{
	Scene::Scene(uint32 Width, uint32 Height, D3D12RHI* pGfx)
	{
		Initialize(Width, Height, pGfx);
	}

	Scene::~Scene()
	{

	}
	
	void Scene::Initialize(uint32 Width, uint32 Height, D3D12RHI* pGfx)
	{
		m_World = new lde::World();
		Camera = std::make_unique<SceneCamera>(m_World, static_cast<float>(Width / Height));
		m_Gfx = pGfx;
		
		PointLights.reserve(4);
		AddPointLight(XMFLOAT3(-8.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(-5.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(0.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(5.0f, 1.0f, 0.5f));

		AddDirectionalLight();

	}

	void Scene::OnResize(float AspectRatio)
	{
		Camera->OnAspectRatioChange(AspectRatio);
	}
	
	void Scene::DrawScene()
	{
		for (auto& model : Models)
		{
			DrawModel(model);
		}
	}

	void Scene::DrawModel(Model& pModel)
	{
		auto* commandList	= m_Gfx->Device->GetGfxCommandList();
		auto& transform		= pModel.GetComponent<TransformComponent>();

		const auto& WVP		= transform.WorldMatrix * Camera->GetViewProjection();
		auto* constBuffer	= m_Gfx->Device->GetConstantBuffer(pModel.ConstBuffer);

		cbPerObject update	= { DirectX::XMMatrixTranspose(WVP), DirectX::XMMatrixTranspose(transform.WorldMatrix) };
		constBuffer->Update(&update);

		m_Gfx->BindConstantBuffer(constBuffer, 0);

		commandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& mesh : pModel.StaticMeshes)
		{
			auto* vertexBuffer = m_Gfx->Device->GetBuffer(mesh.VertexBuffer);

			// Push index to current Vertex Buffer.
			commandList->PushConstants(1, 1, &vertexBuffer->ShaderResource.m_Index);
			// Push Material as constants; 64 bytes
			commandList->PushConstants(2, 16, &mesh.Material, 0);

			if (mesh.NumIndices != 0)
			{
				m_Gfx->BindIndexBuffer(mesh.IndexBufferView);
				m_Gfx->DrawIndexed(mesh.NumIndices, 0, 0);
			}
			else // Draw non-indexed
			{
				m_Gfx->Draw(mesh.NumVertices);
			}
		}
	}
	
	void Scene::AddPointLight(XMFLOAT3 Position)
	{
		Entity* newLight = new Entity();
		newLight->Create(m_World);
		newLight->AddComponent<NameComponent>(std::format("Point Light {}", PointLights.size()).c_str());
		newLight->AddComponent<PointLightComponent>(Position);
		
		PointLights.push_back(newLight);
	}

	void Scene::AddDirectionalLight(XMFLOAT3 Direction)
	{
		Entity* newLight = new Entity();
		newLight->Create(m_World);
		newLight->AddComponent<NameComponent>(std::format("Directional Light {}", DirectionalLights.size()).c_str());
		newLight->AddComponent<DirectionalLightComponent>(Direction);

		DirectionalLights.push_back(newLight);
	}

} // namespace lde
