#include "Components/Components.hpp"
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
		for (auto& model : m_Models)
		{
			model.reset();
		}
	}
	
	void Scene::Initialize(uint32 Width, uint32 Height, D3D12RHI* pGfx)
	{
		m_World = new lde::World();
		m_Camera = std::make_unique<SceneCamera>(m_World, static_cast<float>((float)Width / (float)Height));
		m_Camera->InitializeInputs();
		m_Gfx = pGfx;
		
		PointLights.reserve(4);
		AddPointLight(XMFLOAT3(-8.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(-5.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(0.0f, 1.0f, 0.5f));
		AddPointLight(XMFLOAT3(5.0f, 1.0f, 0.5f));

		//AddDirectionalLight();

	}

	void Scene::OnResize(float AspectRatio)
	{
		m_Camera->OnAspectRatioChange(AspectRatio);
	}
	
	void Scene::DrawScene()
	{
		for (auto& model : m_Models)
		{
			DrawModel(*model);
		}
	}

	void Scene::DrawModel(Model& pModel)
	{
		auto* commandList = m_Gfx->Device->GetGfxCommandList();
		commandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		auto* indexBuffer	= m_Gfx->Device->GetBuffer(pModel.GetMesh()->IndexBuffer);
		auto* vertexBuffer	= m_Gfx->Device->GetBuffer(pModel.GetMesh()->VertexBuffer);
		auto* constBuffer	= m_Gfx->Device->GetConstantBuffer(pModel.GetMesh()->ConstBuffer);

		auto& transform = pModel.GetComponent<TransformComponent>();

		if (indexBuffer->GetDesc().Count != 0)
		{
			m_Gfx->BindIndexBuffer(pModel.GetMesh()->IndexView);
		}

		struct meshVertex
		{
			uint32 index;
			uint32 offset;
		} vertex{ .index = vertexBuffer->GetSRVIndex(), .offset = 0 };

		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12MeshShaders/src/MeshletRender/D3D12MeshletRender.cpp#L377

	#if MESH_SHADING

		//commandList->Get()->SetGraphicsRootShaderResourceView(2, vertexBuffer->GetGpuAddress());
		//commandList->Get()->SetGraphicsRootShaderResourceView(3, mesh.MeshletResource->GetGPUVirtualAddress());
		//commandList->Get()->SetGraphicsRootShaderResourceView(4, mesh.UniqueVertexIndexResource->GetGPUVirtualAddress());
		//auto trianglesBuffer = m_Gfx->Device->Buffers().at(pModel.TrianglesBuffer);
		//commandList->Get()->SetGraphicsRootShaderResourceView(5, pModel.TrianglesBuffer->GetGPUVirtualAddress());


		//commandList->Get()->SetComputeRoot32BitConstant(1, mesh.IndexCount, 0);
		commandList->Get()->SetGraphicsRootShaderResourceView(2, vertexBuffer->GetGpuAddress());
		auto* meshletBuffer = m_Gfx->Device->Buffers.at(pModel.MeshletBuffer);
		commandList->Get()->SetGraphicsRootShaderResourceView(3, meshletBuffer->GetGpuAddress());
		auto* vertsBuffer = m_Gfx->Device->Buffers.at(pModel.UniqueVertexIBBuffer);
		commandList->Get()->SetGraphicsRootShaderResourceView(4, vertsBuffer->GetGpuAddress());
		//auto* trianglesBuffer = m_Gfx->Device->Buffers().at(pModel.TrianglesBuffer);
		commandList->Get()->SetGraphicsRootShaderResourceView(5, indexBuffer->GetGpuAddress());
		//pModel.Triangles.
		commandList->DispatchMesh(pModel.Meshlets.size(), 1, 1);


		for (uint32 i = 0; i < pModel.GetMesh()->Submeshes.size(); i++)
		{
			auto& mesh = pModel.GetMesh()->Submeshes.at(i);

			struct indices
			{
				uint32 vertex;
				uint32 index;
				uint32 meshlet;
				uint32 prims;
			};
			
			

			/*
			commandList->Get()->SetGraphicsRootShaderResourceView(2, mesh.VertexResources[0]->GetGPUVirtualAddress());
			commandList->Get()->SetGraphicsRootShaderResourceView(3, mesh.MeshletResource->GetGPUVirtualAddress());
			commandList->Get()->SetGraphicsRootShaderResourceView(4, mesh.UniqueVertexIndexResource->GetGPUVirtualAddress());
			commandList->Get()->SetGraphicsRootShaderResourceView(5, mesh.PrimitiveIndexResource->GetGPUVirtualAddress());
			*/
		}
		

	#else
		for (uint32 i = 0; i < pModel.GetMesh()->Submeshes.size(); i++)
		{
			auto& mesh = pModel.GetMesh()->Submeshes.at(i);

			//const auto WVP = mesh.Matrix * transform.WorldMatrix * m_Camera->GetViewProjection();
			const auto WVP = transform.WorldMatrix * m_Camera->GetViewProjection();

			cbPerObject update = { XMMatrixTranspose(WVP), DirectX::XMMatrixTranspose(transform.WorldMatrix) };
			constBuffer->Update(&update);
			m_Gfx->BindConstantBuffer(constBuffer, 0);

			vertex.offset = mesh.BaseVertex;
			commandList->PushConstants(1, 2, &vertex);
			// Push Material as constants; 64 bytes
			commandList->PushConstants(2, 16, &mesh.Material, 0);

			if (indexBuffer->GetDesc().Count != 0)
			{
				m_Gfx->DrawIndexed(mesh.IndexCount, mesh.BaseIndex, mesh.BaseVertex);
			}
			else // Draw non-indexed
			{
				m_Gfx->Draw(mesh.VertexCount);
			}
		}
	#endif
	}
	
	void Scene::AddModel(std::string_view Filepath)
	{
		m_Models.emplace_back(std::make_unique<Model>(m_Gfx, Filepath, m_World));
	}

	void Scene::AddPointLight(XMFLOAT3 Position)
	{
		Entity* newLight = new Entity();
		newLight->Create(m_World);
		newLight->AddComponent<TagComponent>(std::format("Point Light {}", PointLights.size()).c_str());
		newLight->AddComponent<PointLightComponent>(Position);
		
		PointLights.push_back(newLight);
	}

	void Scene::AddDirectionalLight(XMFLOAT3 Direction)
	{
		Entity* newLight = new Entity();
		newLight->Create(m_World);
		newLight->AddComponent<TagComponent>(std::format("Directional Light {}", DirectionalLights.size()).c_str());
		newLight->AddComponent<DirectionalLightComponent>(Direction);
		//newLight->AddComponent<DirectionalLightShadowMap>();

		DirectionalLights.push_back(newLight);
	}

} // namespace lde
