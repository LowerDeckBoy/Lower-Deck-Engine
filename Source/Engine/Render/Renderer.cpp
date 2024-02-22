#include "Scene/Scene.hpp"
#include "Renderer.hpp"

namespace lde
{
	bool Renderer::bVSync = true;

	Renderer::Renderer(RHI::D3D12Context* pGfx)
		: m_Gfx(pGfx)
	{
		Initialize();
	}

	Renderer::~Renderer()
	{
		Release();
	}

	void Renderer::Initialize()
	{
		m_ShaderManager		= std::make_unique<ShaderManager>();
		m_TextureManager	= std::make_unique<TextureManager>();
		m_AssetManager		= std::make_unique<AssetManager>();
		m_MipGenerator		= std::make_unique<MipGenerator>();
		m_MipGenerator->Initialize(m_Gfx);

		m_GBufferPass = new GBufferPass(m_Gfx);

		//BuildRootSignatures();
		//BuildPipelines();

	}

	void Renderer::BeginFrame()
	{
		m_Gfx->OpenList(m_Gfx->GraphicsCommandList);

		//SetViewport();
		//TransitToRender();
	}

	void Renderer::EndFrame()
	{
	}

	void Renderer::RecordCommands()
	{
		SetHeaps();
		m_Gfx->SetRootSignature(&m_Gfx->GlobalRootSignature);

		m_GBufferPass->Render(m_ActiveScene);
		m_Gfx->SetViewport();
		m_Gfx->SetRenderTarget();
		m_Gfx->ClearRenderTarget();
		m_Gfx->ClearDepthStencil();

	}

	void Renderer::Update()
	{
	}

	void Renderer::Render()
	{
		m_Gfx->BeginFrame();
		
		//m_Gfx->RecordCommandLists();
		//m_Gfx->Update();
		//m_Gfx->Render();
		
		//m_Gfx->EndFrame();
		//m_Gfx->Present();

		//BeginFrame();

		RecordCommands();

		EndFrame();
	}

	void Renderer::Present()
	{
		m_Gfx->Present(bVSync);
	}

	void Renderer::SetHeaps()
	{
		m_Gfx->GraphicsCommandList->Get()->SetDescriptorHeaps(1, m_Gfx->Heap->GetAddressOf());
	}

	void Renderer::SetScene(Scene* pScene)
	{
		m_ActiveScene = pScene;
	}

	void Renderer::OnResize(uint32 Width, uint32 Height)
	{
		m_Gfx->OnResize(Width, Height);

		m_GBufferPass->OnResize(Width, Height);

		m_Gfx->Device->WaitForGPU();
	}

	void Renderer::Release()
	{
		delete m_GBufferPass;

		// Release gathered Textures
		TextureManager::GetInstance().Release();
		MipGenerator::GetInstance().Release();
		m_MipGenerator.reset();
		//m_TextureManager->Release();
		m_AssetManager.reset();
		m_ShaderManager.reset();
	}

	void Renderer::BuildRootSignatures()
	{
		D3D12_ROOT_SIGNATURE_FLAGS rootFlags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		// Base Root Signature
		{
			std::vector<CD3DX12_ROOT_PARAMETER1> parameters(3);
			// Per Object Matrices
			parameters.at(0).InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
			// Camera Buffer
			parameters.at(1).InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
			parameters.at(2).InitAsConstants(16, 2, 0, D3D12_SHADER_VISIBILITY_ALL);

			std::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
			samplers.at(0) = RHI::D3D12Utility::CreateStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);

			m_BaseRootSignature.Create(m_Gfx->Device.get(), parameters, samplers, rootFlags, L"Base Root Signature");
		}
	}

	void Renderer::BuildPipelines()
	{
		auto* psoBuilder = new RHI::D3D12PipelineStateBuilder(m_Gfx->Device.get());

		// Base Pipeline
		{
			psoBuilder->SetVertexShader("Shaders/GBuffer.hlsl", L"VSmain");
			psoBuilder->SetPixelShader("Shaders/GBuffer.hlsl", L"PSmain");
			psoBuilder->EnableDepth(true);
			std::vector<DXGI_FORMAT> formats(6, DXGI_FORMAT_R8G8B8A8_UNORM);
			psoBuilder->SetRenderTargetFormats(formats);

			RHI::DX_CALL(psoBuilder->Build(m_BasePipeline, &m_BaseRootSignature, "Base Pipeline"));
			psoBuilder->Reset();
		}
	}

	uint64 Renderer::GetRenderTarget()
	{
		switch (SelectedRenderTarget)
		{
		case RenderOutput::eDepth:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eDepth).SRV.GetGpuHandle().ptr;
		case RenderOutput::eBaseColor:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eBaseColor).SRV.GetGpuHandle().ptr;
		case RenderOutput::eTexCoords:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eTexCoords).SRV.GetGpuHandle().ptr;
		case RenderOutput::eNormal:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eNormal).SRV.GetGpuHandle().ptr;
		case RenderOutput::eMetalRoughness:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eMetalRoughness).SRV.GetGpuHandle().ptr;
		case RenderOutput::eEmissive:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eEmissive).SRV.GetGpuHandle().ptr;
		case RenderOutput::eWorldPosition:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eWorldPosition).SRV.GetGpuHandle().ptr;
		//case RenderOutput::eShadows:
		//	break;
		//case RenderOutput::eAmbientOcclusion:
		//	break;
		//case RenderOutput::eRaytracing:
		//	break;
		//case RenderOutput::eScene:
		//	break;
		//case RenderOutput::Count:
		//	break;
		//default:
		//	break;
		}

		return m_GBufferPass->GetRenderTargets().at(GBuffers::eBaseColor).SRV.GetGpuHandle().ptr;
	}

} // namespace lde
