#pragma once

//
//
//
//

#include "Entity.hpp"
#include "Model/Model.hpp"
#include "SceneCamera.hpp"
#include <Core/CoreMinimal.hpp>
//#include <memory>

namespace lde
{
	class D3D12RHI;

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
		Scene(uint32 Width, uint32 Height, RHI::D3D12RHI* pContext);
		~Scene();
	
		void Initialize(uint32 Width, uint32 Height, RHI::D3D12RHI* pContext);
	
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
		
		std::string SceneName = "Default scene";
		std::unique_ptr<SceneCamera> m_Camera;
		
		std::vector<Entity> Lights;
	
	private:
		lde::World* m_World = nullptr;
	
		std::vector<std::unique_ptr<Model>> m_Models;
	
		RHI::D3D12RHI* m_Gfx = nullptr;
	};

} // namespace lde
