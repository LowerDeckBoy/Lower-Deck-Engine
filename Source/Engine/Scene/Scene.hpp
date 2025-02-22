#pragma once

#include "Entity.hpp"
#include "Model/Model.hpp"
#include "SceneCamera.hpp"
#include <Core/CoreMinimal.hpp>

namespace lde
{
	class D3D12RHI;

	class Scene
	{
	public:
		Scene(uint32 Width, uint32 Height, D3D12RHI* pContext);
		~Scene();
	
		void Initialize(uint32 Width, uint32 Height, D3D12RHI* pContext);
	
		void OnResize(float AspectRatio);
	
		World* World()
		{
			return m_World;
		}
	
		entt::registry* Registry() const
		{
			return m_World->Registry();
		}
	
		inline Entity NewEntity()
		{
			return Entity(m_World);
		}
		
		// Draw all Models in this scene.
		void DrawScene();

		void DrawModel(Model& pModel);

		SceneCamera* GetCamera()
		{
			return Camera.get();
		}

		std::string SceneName = "Default scene";

		std::unique_ptr<SceneCamera> Camera;
		
		std::vector<Entity*> PointLights;
		std::vector<Entity*> DirectionalLights;
	
		void AddPointLight(DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
		void AddDirectionalLight(DirectX::XMFLOAT3 Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f));

		std::vector<Model> Models;

	private:
		lde::World* m_World = nullptr;
		D3D12RHI* m_Gfx = nullptr;

	};

} // namespace lde
