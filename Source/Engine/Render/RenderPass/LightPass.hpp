#pragma once

#include <Core/CoreTypes.hpp>
#include <RHI/Buffer.hpp>
#include <Scene/Components/LightComponent.hpp>
#include <memory>

namespace lde
{
	class D3D12RHI;
	class D3D12RenderTexture;

	class SceneCamera;
	class GBufferPass;
	class Skybox;
	class Scene;

	struct LightData
	{
		// Global light
		DirectionalLightComponent	Directional;
		PointLightComponent			Light[4];
	};

	class LightPass
	{
	public:
		LightPass(D3D12RHI* pGfx);
		~LightPass();

		void Render(SceneCamera* pCamera, GBufferPass* pGBuffer, Skybox* pSkybox, Scene* pScene);

		void Resize(uint32 Width, uint32 Height);

		void Release();

		D3D12RenderTexture* GetRenderTexture() { return m_Texture.get(); }

		std::vector<PointLightComponent>& GetPointLights() { return m_PointLights; }

	private:
		std::unique_ptr<D3D12RenderTexture> m_Texture;
		D3D12RHI* m_Gfx = nullptr;
	
		BufferHandle m_SceneConstBuffer = 0;
		SceneData m_SceneData{};

		BufferHandle m_SceneLighting = 0;
		DirectionalLightComponent m_DirLight{};
		LightData m_LightsData{};

		void Create(D3D12RHI* pGfx);

		std::vector<PointLightComponent> m_PointLights{};

	};
} // namespace lde
