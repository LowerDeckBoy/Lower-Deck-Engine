#include "Scene/Scene.hpp"
#include "Renderer.hpp"

namespace lde
{
	bool Renderer::bVSync = true;

	Renderer::Renderer(D3D12RHI* pGfx)
		: m_Gfx(pGfx)
	{
		Initialize();
	}

	Renderer::Renderer(D3D12RHI* pGfx, Scene* pScene)
		: m_Gfx(pGfx)
	{
		m_ShaderCompiler = std::make_unique<ShaderCompiler>();
		m_TextureManager = std::make_unique<TextureManager>();
		m_AssetManager   = std::make_unique<AssetManager>();

		m_TextureManager->Initialize(m_Gfx);

		m_Skybox = std::make_unique<Skybox>();
		SetScene(pScene);
		Initialize();

	}

	Renderer::~Renderer()
	{
		Release();
	}

	void Renderer::Initialize()
	{
		BuildRootSignatures();
		BuildPipelines();
		
		//SceneImage.Initialize(m_Gfx, DXGI_FORMAT_R8G8B8A8_UNORM, "Scene final image");
		SceneImage.Initialize(m_Gfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "Scene final image");

		m_IBL = std::make_unique<ImageBasedLighting>(m_Gfx, m_Skybox.get(), "Assets/Textures/newport_loft.hdr");
		//m_IBL = std::make_unique<ImageBasedLighting>(m_Gfx, m_Skybox.get(), "Assets/Textures/environment.hdr");
		//m_IBL = std::make_unique<ImageBasedLighting>(m_Gfx, m_Skybox.get(), "Assets/Textures/kloofendal_48d_partly_cloudy_puresky_2k.hdr");
		//m_IBL = std::make_unique<ImageBasedLighting>(m_Gfx, m_Skybox.get(), "Assets/Textures/san_giuseppe_bridge_4k.hdr");
		//m_IBL = std::make_unique<ImageBasedLighting>(m_Gfx, m_Skybox.get(), "Assets/Textures/animestyled_hdr.hdr");
		
		m_GBufferPass	= new GBufferPass(m_Gfx);
		m_LightPass		= new LightPass(m_Gfx);
		m_SkyPass		= new SkyPass(m_Gfx);
		
	}

	void Renderer::BeginFrame()
	{	
		m_Gfx->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		
	}

	void Renderer::RecordCommands()
	{
		m_Gfx->Device->GetGfxCommandList()->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Gfx->ClearDepthStencil();

		// G-Buffer
		{
			m_GBufferPass->Render(m_ActiveScene);
		}

		m_Gfx->TransitResource(SceneImage.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_Gfx->SetRenderTarget(SceneImage.GetRTV().GetCpuHandle(), &m_Gfx->SceneDepth->DSV().GetCpuHandle());
		m_Gfx->ClearRenderTarget(SceneImage.GetRTV().GetCpuHandle());

	#if MESH_SHADING
		m_Gfx->SetRootSignature(&m_MeshletRS);
		m_Gfx->SetPipeline(&m_MeshletPSO);
		m_ActiveScene->DrawScene();
	#endif

		// Light Pass
		{
			m_Gfx->SetRootSignature(&m_LightRS);
			m_Gfx->SetPipeline(&m_LightPSO);
			m_LightPass->Render(m_ActiveScene->GetCamera(), m_GBufferPass, m_Skybox.get(), m_ActiveScene);
		}	

		// Sky Pass
		{
			m_Gfx->SetRootSignature(&m_SkyboxRS);
			m_Gfx->SetPipeline(&m_SkyboxPSO);
			m_Skybox->Draw(-1, m_ActiveScene->GetCamera()); 
		}
		m_Gfx->TransitResource(SceneImage.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		

		m_Gfx->SetViewport();
		m_Gfx->SetMainRenderTarget();
		m_Gfx->ClearMainRenderTarget();

	}

	void Renderer::Update()
	{
	}

	void Renderer::Render()
	{
		BeginFrame();

		RecordCommands();

		EndFrame();
	}

	void Renderer::Present()
	{
		m_Gfx->Present(bVSync);
	}

	void Renderer::SetScene(Scene* pScene)
	{
		m_ActiveScene = pScene;

		m_Skybox->Create(m_Gfx, m_ActiveScene->World(), "Assets/Textures/newport_loft.hdr");

	}

	void Renderer::OnResize(uint32 Width, uint32 Height)
	{
		m_Gfx->OnResize(Width, Height);

		m_GBufferPass->Resize(Width, Height);
		m_LightPass->Resize(Width, Height);
		m_SkyPass->Resize(Width, Height);
		SceneImage.OnResize(Width, Height);

	}

	void Renderer::Release()
	{
	#if RAYTRACING
		delete RaytracingCtx;
	#endif

		delete m_SkyPass;
		delete m_LightPass;
		delete m_GBufferPass;

		// Release gathered Textures
		TextureManager::GetInstance().Release();
		m_AssetManager.reset();
		m_ShaderCompiler.reset();
	}

	void Renderer::BuildRootSignatures()
	{
		// LightPass Root Signature
		{
			// Scene data	
			m_LightRS.AddCBV(0);
			// Lighting data	
			m_LightRS.AddCBV(1);
			// GBuffer indices
			m_LightRS.AddConstants(7, 2);
			// Image Based Lighting indices
			m_LightRS.AddConstants(3, 3);
			// Texture sampling
			m_LightRS.AddStaticSampler(0, 0, D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
			// Specular BRDF sampling
			m_LightRS.AddStaticSampler(1, 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

			DX_CALL(m_LightRS.Build(m_Gfx->Device.get(), PipelineType::eGraphics, "LightPass Root Signature"));
			
		}

		// Skyboox Root Signature
		{
			// Scene data
			m_SkyboxRS.AddCBV(0);
			// Skybox texture to draw
			m_SkyboxRS.AddConstants(1, 1);
			m_SkyboxRS.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_LESS_EQUAL);
			DX_CALL(m_SkyboxRS.Build(m_Gfx->Device.get(), PipelineType::eGraphics, "Skybox Root Signature"));
		}

	}

	void Renderer::BuildPipelines()
	{
		D3D12PipelineStateBuilder psoBuilder(m_Gfx->Device.get());

		// Light Pass
		{
			psoBuilder.SetVS("Shaders/Deferred/PBR.hlsl", L"VSmain");
			psoBuilder.SetPS("Shaders/Deferred/PBR.hlsl", L"PSmain");
			psoBuilder.EnableDepth(false);
			psoBuilder.SetCullMode(CullMode::eNone);
			std::array<DXGI_FORMAT, 1> formats{ DXGI_FORMAT_R32G32B32A32_FLOAT };
			psoBuilder.SetRenderTargetFormats(formats);

			DX_CALL(psoBuilder.Build(m_LightPSO, &m_LightRS));
			psoBuilder.Reset();
		}

		// Skybox
		{
			psoBuilder.SetVS("Shaders/Sky/Skybox.hlsl", L"VSmain");
			psoBuilder.SetPS("Shaders/Sky/Skybox.hlsl", L"PSmain");
			psoBuilder.EnableDepth(true);
			std::vector<DXGI_FORMAT> formats{ DXGI_FORMAT_R32G32B32A32_FLOAT };
			psoBuilder.SetRenderTargetFormats(formats);

			DX_CALL(psoBuilder.Build(m_SkyboxPSO, &m_SkyboxRS));
			psoBuilder.Reset();
		}
	}

	uint64 Renderer::GetRenderTarget()
	{
		switch (SelectedRenderTarget)
		{
		case RenderOutput::eShaded:
			return SceneImage.GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eDepth:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eDepth).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eBaseColor:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eBaseColor).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eTexCoords:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eTexCoords).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eNormal:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eNormal).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eMetalRoughness:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eMetalRoughness).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eEmissive:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eEmissive).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eWorldPosition:
			return m_GBufferPass->GetRenderTargets().at(GBuffers::eWorldPosition).GetSRV().GetGpuHandle().ptr;
		case RenderOutput::eSkybox:
			return m_SkyPass->GetRenderTexture()->GetSRV().GetGpuHandle().ptr;
		}

		return SceneImage.GetSRV().GetGpuHandle().ptr;
	}

} // namespace lde
