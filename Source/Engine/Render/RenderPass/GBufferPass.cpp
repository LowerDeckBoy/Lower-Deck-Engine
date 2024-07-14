#include "RHI/D3D12/D3D12PipelineState.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "RHI/D3D12/D3D12CommandList.hpp"
#include "GBufferPass.hpp"
#include "RHI/D3D12/D3D12RHI.hpp"
#include "Scene/Scene.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"

namespace lde
{
	GBufferPass::GBufferPass(RHI::D3D12RHI* pGfx)
	{
		m_Gfx = pGfx;

		m_RenderTargets.at(GBuffers::eDepth).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "GBuffer Depth");
		m_RenderTargets.at(GBuffers::eBaseColor).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "GBuffer BaseColor");
		m_RenderTargets.at(GBuffers::eTexCoords).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "GBuffer TexCoords");
		m_RenderTargets.at(GBuffers::eNormal).Initialize(m_Gfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "GBuffer Normal");
		m_RenderTargets.at(GBuffers::eMetalRoughness).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "GBuffer MetalRoughness");
		m_RenderTargets.at(GBuffers::eEmissive).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "GBuffer Emissive");
		m_RenderTargets.at(GBuffers::eWorldPosition).Initialize(m_Gfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "GBuffer WorldPosition");
		
		// Root Signature
		{
			m_RootSignature.AddCBV(0);			 // Per Object Matrices
			m_RootSignature.AddConstants(2, 1);  // Vertex vertices and offset
			m_RootSignature.AddConstants(16, 2); // Texture indices and properties
			m_RootSignature.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
			m_RootSignature.Build(m_Gfx->Device.get(), RHI::PipelineType::eGraphics, "GBuffer Root Signature");
		}

		// Pipeline State
		RHI::D3D12PipelineStateBuilder psoBuilder(m_Gfx->Device.get());

		// Base Pipeline
		{
			psoBuilder.SetVS("Shaders/Deferred/GBuffer.hlsl", L"VSmain");
			psoBuilder.SetPS( "Shaders/Deferred/GBuffer.hlsl", L"PSmain");
			psoBuilder.EnableDepth(true);
			psoBuilder.SetCullMode(RHI::CullMode::eBack);
			std::array<DXGI_FORMAT, (usize)GBuffers::COUNT> formats =
			{
				m_RenderTargets.at(GBuffers::eDepth).GetFormat(),
				m_RenderTargets.at(GBuffers::eBaseColor).GetFormat(),
				m_RenderTargets.at(GBuffers::eTexCoords).GetFormat(),
				m_RenderTargets.at(GBuffers::eNormal).GetFormat(),
				m_RenderTargets.at(GBuffers::eMetalRoughness).GetFormat(),
				m_RenderTargets.at(GBuffers::eEmissive).GetFormat(),
				m_RenderTargets.at(GBuffers::eWorldPosition).GetFormat()
			};
			psoBuilder.SetRenderTargetFormats(formats);

			RHI::DX_CALL(psoBuilder.Build(m_PipelineState, &m_RootSignature));
			psoBuilder.Reset();
		}

		m_CommandSignature = new RHI::D3D12CommandSignature();
		m_CommandSignature->AddCBV(0);			// 8 bytes
		m_CommandSignature->AddConstant(1, 2);	// 8 bytes
		m_CommandSignature->AddConstant(2, 16);	// 64 bytes
		m_CommandSignature->AddDrawIndexed();	// 20 bytes
		//RHI::DX_CALL(m_CommandSignature->Create(m_Gfx->Device.get(), &m_RootSignature));

	}

	GBufferPass::~GBufferPass()
	{
		Release();
	}

	void GBufferPass::Render(Scene* pScene)
	{
		m_Gfx->SetRootSignature(&m_RootSignature);
		m_Gfx->SetPipeline(&m_PipelineState);

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
		for (auto& renderTarget : m_RenderTargets)
		{
			m_Gfx->TransitResource(renderTarget.second.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_Gfx->ClearRenderTarget(renderTarget.second.GetRTV().GetCpuHandle());
			rtvs.push_back(renderTarget.second.GetRTV().GetCpuHandle());
		}

		//m_Gfx->ClearDepthStencil();
		m_Gfx->SetRenderTargets(rtvs, m_Gfx->SceneDepth->DSV().GetCpuHandle());

		pScene->DrawScene();

		for (auto& rtv : m_RenderTargets)
		{
			m_Gfx->TransitResource(rtv.second.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		
	}

	void GBufferPass::Resize(uint32 Width, uint32 Height)
	{
		for (auto& renderTarget : m_RenderTargets)
		{
			renderTarget.second.OnResize(Width, Height);
		}
	}

	void GBufferPass::Release()
	{
		//for (auto& renderTarget : m_RenderTargets)
		//{
		//	//renderTarget.second.
		//}
		m_RootSignature.Release();
		m_CommandSignature->Release();
	}

	std::array<int, 7> GBufferPass::GetTextureIndices()
	{
		std::array<int, 7> indices{};
		int i = 0;
		for (auto it = m_RenderTargets.begin(); it != m_RenderTargets.end(); ++it)
		{
			indices.at(i) = it->second.GetSRV().Index();
			i++;
		}

		return indices;
	}
}
