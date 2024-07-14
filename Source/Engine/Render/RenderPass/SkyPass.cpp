#include "Graphics/Skybox.hpp"
#include "RHI/D3D12/D3D12Texture.hpp"
#include "SkyPass.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"

namespace lde
{
	SkyPass::SkyPass(RHI::D3D12RHI* pRHI)
		: m_Gfx(pRHI)
	{
		m_Texture = std::make_unique<RHI::D3D12RenderTexture>();
		m_Texture->Initialize(m_Gfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "SkyPass RenderTexture");
	}

	SkyPass::~SkyPass()
	{
		m_Texture.reset();
	}

	void SkyPass::Render(Skybox* pSkybox, SceneCamera* pCamera)
	{
		m_Gfx->TransitResource(m_Texture->Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto& rtvHandle = m_Texture->GetRTV().GetCpuHandle();
		auto& depthHandle = m_Gfx->SceneDepth->DSV().GetCpuHandle();

		m_Gfx->ClearRenderTarget(rtvHandle);
		//m_Gfx->ClearDepthStencil();
		m_Gfx->SetRenderTarget(rtvHandle, &depthHandle);

		pSkybox->Draw(-1, pCamera);
		m_Gfx->TransitResource(m_Texture->Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

	void SkyPass::Resize(uint32 Width, uint32 Height)
	{
		m_Texture->OnResize(Width, Height);
	}

} // namespace lde
