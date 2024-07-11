#pragma once

#include <Core/CoreTypes.hpp>
#include <RHI/Buffer.hpp>
#include <Scene/Components/LightComponent.hpp>
#include <memory>

namespace lde::RHI
{
	class D3D12RHI;
	class D3D12RenderTexture;
}

namespace lde
{
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
		LightPass(RHI::D3D12RHI* pGfx);
		~LightPass();

		void Render(SceneCamera* pCamera, GBufferPass* pGBuffer, Skybox* pSkybox, Scene* pScene);

		void Resize(uint32 Width, uint32 Height);

		void Release();

		RHI::D3D12RenderTexture* GetRenderTexture() { return m_Texture.get(); }

		std::vector<PointLightComponent>& GetPointLights() { return m_PointLights; }

	private:
		std::unique_ptr<RHI::D3D12RenderTexture> m_Texture;
		RHI::D3D12RHI* m_Gfx = nullptr;
	
		BufferHandle m_SceneConstBuffer = 0;
		RHI::SceneData m_SceneData{};

		BufferHandle m_SceneLighting = 0;
		DirectionalLightComponent m_DirLight{};
		LightData m_LightsData{};

		void Create(RHI::D3D12RHI* pGfx);

		std::vector<PointLightComponent> m_PointLights{};

	};
} // namespace lde
