#include "RHI/D3D12/D3D12PipelineState.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "GBufferPass.hpp"
#include "RHI/D3D12/D3D12Context.hpp"
#include "Scene/Scene.hpp"

namespace lde
{
	GBufferPass::GBufferPass(RHI::D3D12Context* pGfx)
	{
		m_Gfx = pGfx;

		m_RenderTargets.at(GBuffers::eDepth).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Depth");
		m_RenderTargets.at(GBuffers::eBaseColor).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer BaseColor");
		m_RenderTargets.at(GBuffers::eTexCoords).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer TexCoords");
		m_RenderTargets.at(GBuffers::eNormal).Initialize(m_Gfx, DXGI_FORMAT_R16G16B16A16_FLOAT, L"GBuffer Normal");
		m_RenderTargets.at(GBuffers::eMetalRoughness).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer MetalRoughness");
		m_RenderTargets.at(GBuffers::eEmissive).Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, L"GBuffer Emissive");
		m_RenderTargets.at(GBuffers::eWorldPosition).Initialize(m_Gfx, DXGI_FORMAT_R32G32B32A32_FLOAT, L"GBuffer WorldPosition");

		D3D12_ROOT_SIGNATURE_FLAGS rootFlags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		// Root Signature
		{
			std::vector<CD3DX12_ROOT_PARAMETER1> parameters(3);
			// Per Object Matrices
			parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
			// Vertex vertices and offset
			parameters.at(1).InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
			// Texture indices
			parameters.at(2).InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_ALL);

			std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
			samplers.at(0) = RHI::D3D12Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);

			m_RootSignature.Create(m_Gfx->Device.get(), parameters, samplers, rootFlags, L"GBuffer Root Signature");
		}

		// Pipeline State
		{
			auto* psoBuilder = new RHI::D3D12PipelineStateBuilder(m_Gfx->Device.get());

			// Base Pipeline
			{
				psoBuilder->SetVertexShader("Shaders/GBuffer.hlsl", L"VSmain");
				psoBuilder->SetPixelShader("Shaders/GBuffer.hlsl", L"PSmain");
				psoBuilder->EnableDepth(true);
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
				psoBuilder->SetRenderTargetFormats(formats);

				RHI::DX_CALL(psoBuilder->Build(m_PipelineState, &m_RootSignature, "GBuffer PSO"));
				psoBuilder->Reset();
			}
		}
	}

	void GBufferPass::Render(Scene* pScene)
	{
		m_Gfx->SetRootSignature(&m_RootSignature);
		m_Gfx->SetPipeline(&m_PipelineState);

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvs;
		for (auto& renderTarget : m_RenderTargets)
		{
			m_Gfx->TransitResource(renderTarget.second.Resource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_Gfx->ClearRenderTarget(renderTarget.second.RTV.GetCpuHandle());
			rtvs.push_back(renderTarget.second.RTV.GetCpuHandle());
		}

		m_Gfx->SetRenderTargets(rtvs, m_Gfx->SceneDepth->DSV().GetCpuHandle());

		pScene->DrawScene();

		for (auto& rtv : m_RenderTargets)
		{
			m_Gfx->TransitResource(rtv.second.Resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		
	}

	void GBufferPass::OnResize(uint32 Width, uint32 Height)
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
	}
}
