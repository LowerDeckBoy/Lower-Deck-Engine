#pragma once

namespace lde::RHI
{
	class D3D12RHI;
	class D3D12RenderTexture;
}

namespace lde
{
	class Skybox;
	class SceneCamera;

	class SkyPass
	{
	public:
		SkyPass(RHI::D3D12RHI* pRHI);
		~SkyPass();

		void Render(Skybox* pSkybox, SceneCamera* pCamera);

		void Resize(uint32 Width, uint32 Height);

		RHI::D3D12RenderTexture* GetRenderTexture() { return m_Texture.get(); }

	private:
		std::unique_ptr<RHI::D3D12RenderTexture> m_Texture;
		RHI::D3D12RHI* m_Gfx = nullptr;

	};
} // namespace lde
