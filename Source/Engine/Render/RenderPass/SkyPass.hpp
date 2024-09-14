#pragma once

namespace lde
{
	class D3D12RHI;
	class D3D12RenderTexture;
	class Skybox;
	class SceneCamera;

	class SkyPass
	{
	public:
		SkyPass(D3D12RHI* pRHI);
		~SkyPass();

		void Render(Skybox* pSkybox, SceneCamera* pCamera);

		void Resize(uint32 Width, uint32 Height);

		D3D12RenderTexture* GetRenderTexture() { return m_Texture.get(); }

	private:
		std::unique_ptr<D3D12RenderTexture> m_Texture;
		D3D12RHI* m_Gfx = nullptr;

	};
} // namespace lde
