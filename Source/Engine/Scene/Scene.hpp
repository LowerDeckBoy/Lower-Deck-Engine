#pragma once

//
//
//
//

#include <Core/CoreMinimal.hpp>
#include <memory>

//#include "World.hpp"
#include "Entity.hpp"
#include "Model/Model.hpp"
#include "SceneCamera.hpp"

namespace lde
{
	class D3D12Context;

	struct SceneData
	{
		XMFLOAT4 CameraPosition;
		XMMATRIX CameraView;
		XMMATRIX CameraProjection;

		uint32 Width;
		uint32 Height;
		int32 DirLightsCount;
		int32 PointLightsCount;
	};

	class Scene
	{
	public:
		Scene(uint32 Width, uint32 Height, RHI::D3D12Context* pContext);
		~Scene();
	
		void Initialize(uint32 Width, uint32 Height, RHI::D3D12Context* pContext);
	
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
	
		/// @brief 
		void DrawScene();
	
		void DrawModel(Model& pModel);
	
		const std::vector<std::unique_ptr<Model>>& GetModels() const
		{
			return m_Models;
		}

		SceneCamera* GetCamera()
		{
			return m_Camera.get();
		}
	
		void AddModel(std::string_view Filepath);
		
		//std::shared_ptr<SceneCamera> Camera;
	
		//ECS::Entity m_CameraEntity;
		std::string SceneName{ "Default scene" };
		std::unique_ptr<SceneCamera> m_Camera;
		
		std::vector<Entity> Lights;
	
	private:
		lde::World* m_World = nullptr;
	
		std::vector<std::unique_ptr<Model>> m_Models;
	
		RHI::D3D12Context* m_Gfx = nullptr;
	};

} // namespace lde