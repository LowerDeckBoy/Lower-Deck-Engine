#include "D3D12PipelineState.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"
#include <AgilitySDK/d3dx12/d3dx12_pipeline_state_stream.h>

namespace lde::RHI
{

	D3D12PipelineState::~D3D12PipelineState()
	{
		SAFE_RELEASE(PipelineState);
	}
	
	D3D12PipelineStateBuilder::D3D12PipelineStateBuilder(D3D12Device* pDevice)
		: m_Device(pDevice)
	{
		Reset();
	}
	
	D3D12PipelineStateBuilder::~D3D12PipelineStateBuilder()
	{
		Reset();
	}

	HRESULT D3D12PipelineStateBuilder::Build(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature, const std::string& /* DebugName */)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.NodeMask = DEVICE_NODE;
		desc.pRootSignature = pRootSignature->Get();
		
		// States
		// Rasterizer
		desc.RasterizerState = m_RasterizerDesc;
		desc.RasterizerState.CullMode = m_CullMode;
		desc.RasterizerState.FillMode = m_FillMode;
		desc.RasterizerState.MultisampleEnable = true;
		desc.RasterizerState.AntialiasedLineEnable = false;
		//desc.RasterizerState.DepthBias = 1;
		//desc.RasterizerState.DepthBiasClamp = 1.0f;
		// Depth
		desc.DepthStencilState = m_DepthDesc;
		desc.DSVFormat = m_DepthFormat;
		// Blend
		desc.BlendState = m_BlendDesc;

		desc.SampleMask = UINT_MAX;

		// Set Shaders
		if (m_VertexShader)
		{
			desc.VS = m_VertexShader->Bytecode();
		}
		if (m_PixelShader)
		{
			desc.PS = m_PixelShader->Bytecode();
		}
		if (m_GeometryShader)
		{
			desc.GS = m_GeometryShader->Bytecode();
		}
		if (m_HullShader)
		{
			desc.HS = m_HullShader->Bytecode();
		}
		if (m_TessellationShader)
		{
			//desc.S = m_HullShader->Bytecode();
		}
		if (m_DomainShader)
		{
			desc.DS = m_DomainShader->Bytecode();
		}

		desc.PrimitiveTopologyType = (m_HullShader) ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc = { 1, 0 };

		// Render Targets
		if (!m_RenderTargetFormats.empty())
		{
			desc.NumRenderTargets = static_cast<uint32>(m_RenderTargetFormats.size());
			for (uint32 i = 0; i < static_cast<uint32>(m_RenderTargetFormats.size()); i++)
			{
				desc.RTVFormats[i] = m_RenderTargetFormats.at(i);
			}
		}
		else
		{
			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		OutPipeline.Type = PipelineType::eGraphics;
		OutPipeline.RootSignature = pRootSignature;
		return m_Device->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&OutPipeline.PipelineState));

	}

	HRESULT D3D12PipelineStateBuilder::BuildMesh(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature, const std::string& DebugName)
	{
		struct 
		{
			ID3D12RootSignature*			pRootSignature;
			D3D12_SHADER_BYTECODE			AS;
			D3D12_SHADER_BYTECODE			MS;
			D3D12_SHADER_BYTECODE			PS;
			D3D12_BLEND_DESC				BlendState;
			UINT							SampleMask;
			D3D12_RASTERIZER_DESC			RasterizerState;
			D3D12_DEPTH_STENCIL_DESC		DepthStencilState;
			D3D12_PRIMITIVE_TOPOLOGY_TYPE	PrimitiveTopologyType;
			UINT							NumRenderTargets;
			DXGI_FORMAT						RTVFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
			DXGI_FORMAT						DSVFormat;
			DXGI_SAMPLE_DESC				SampleDesc;
			UINT							NodeMask;
			D3D12_CACHED_PIPELINE_STATE		CachedPSO;
			D3D12_PIPELINE_STATE_FLAGS		Flags;
		} MPSO{};

		MPSO.NodeMask = DEVICE_NODE;

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
		streamDesc.pPipelineStateSubobjectStream = &MPSO;
		streamDesc.SizeInBytes = sizeof(MPSO);

		return m_Device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&OutPipeline.PipelineState));
	}

	void D3D12PipelineStateBuilder::SetVS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_VertexShader) m_VertexShader = nullptr;
		m_VertexShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::eVertex, EntryPoint));
	}
	
	void D3D12PipelineStateBuilder::SetPS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_PixelShader) m_PixelShader = nullptr;
		m_PixelShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::ePixel, EntryPoint));
	}

	void D3D12PipelineStateBuilder::SetGS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_GeometryShader) m_GeometryShader = nullptr;
		m_GeometryShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::eGeometry, EntryPoint));
	}

	void D3D12PipelineStateBuilder::SetHS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_HullShader) m_HullShader = nullptr;
		m_HullShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::eHull, EntryPoint));
	}

	void D3D12PipelineStateBuilder::SetTS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_TessellationShader) m_TessellationShader = nullptr;
		m_TessellationShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::eTessellation, EntryPoint));
	}

	void D3D12PipelineStateBuilder::SetDS(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();
		if (m_DomainShader) m_DomainShader = nullptr;
		m_DomainShader = new Shader(shaderCompiler.Compile(Filepath, ShaderStage::eDomain, EntryPoint));
	}
	
	void D3D12PipelineStateBuilder::SetCullMode(CullMode eMode)
	{
		switch (eMode)
		{
		case CullMode::eBack:
			m_CullMode = D3D12_CULL_MODE_BACK;
			break;
		case CullMode::eFront:
			m_CullMode = D3D12_CULL_MODE_FRONT;
			break;
		case CullMode::eNone:
			m_CullMode = D3D12_CULL_MODE_NONE;
			break;
		}
	}
	
	void D3D12PipelineStateBuilder::SetWireframe(bool bEnable)
	{
		m_FillMode = bEnable ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	}

	void D3D12PipelineStateBuilder::SetDepthFormat(DXGI_FORMAT Format)
	{
		m_DepthFormat = Format;
	}
	
	void D3D12PipelineStateBuilder::EnableDepth(bool bEnable)
	{
		m_DepthDesc.DepthEnable = bEnable;
		m_RasterizerDesc.DepthClipEnable = bEnable;
	}
	
	void D3D12PipelineStateBuilder::SetRenderTargetFormats(const std::span<DXGI_FORMAT>& Formats)
	{
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.insert(m_RenderTargetFormats.begin(), Formats.begin(), Formats.end());
	}
	
	void D3D12PipelineStateBuilder::Reset()
	{
		m_VertexShader = nullptr;
		m_PixelShader  = nullptr;
	
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.shrink_to_fit();
		
		m_RasterizerDesc = {};
		m_FillMode = D3D12_FILL_MODE_SOLID;
		m_CullMode = D3D12_CULL_MODE_BACK;
	
		m_DepthDesc = {};

		// Default Depth Desc
		m_DepthDesc.DepthEnable					= TRUE;
		m_DepthDesc.DepthWriteMask				= D3D12_DEPTH_WRITE_MASK_ALL;
		m_DepthDesc.DepthFunc					= D3D12_COMPARISON_FUNC_LESS;
		m_DepthDesc.StencilEnable				= FALSE;
		m_DepthDesc.StencilReadMask				= D3D12_DEFAULT_STENCIL_READ_MASK;
		m_DepthDesc.StencilWriteMask			= D3D12_DEFAULT_STENCIL_WRITE_MASK;

		// Default Rasterizer Desc
		m_RasterizerDesc.FillMode				= D3D12_FILL_MODE_SOLID;
		m_RasterizerDesc.CullMode				= D3D12_CULL_MODE_BACK;
		m_RasterizerDesc.FrontCounterClockwise	= FALSE;
		m_RasterizerDesc.DepthBias				= D3D12_DEFAULT_DEPTH_BIAS;
		m_RasterizerDesc.DepthBiasClamp			= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		m_RasterizerDesc.SlopeScaledDepthBias	= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		m_RasterizerDesc.DepthClipEnable		= TRUE;
		m_RasterizerDesc.MultisampleEnable		= FALSE;
		m_RasterizerDesc.AntialiasedLineEnable	= FALSE;
		m_RasterizerDesc.ForcedSampleCount		= 0;
		m_RasterizerDesc.ConservativeRaster		= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Default Blend Desc
		m_BlendDesc.AlphaToCoverageEnable		= FALSE;
		m_BlendDesc.IndependentBlendEnable		= FALSE;
		m_BlendDesc.RenderTarget[0].BlendEnable = FALSE;
		m_BlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		m_BlendDesc.RenderTarget[0].SrcBlend	= D3D12_BLEND_ONE;
		m_BlendDesc.RenderTarget[0].DestBlend	= D3D12_BLEND_ZERO;
		m_BlendDesc.RenderTarget[0].BlendOp		= D3D12_BLEND_OP_ADD;
		m_BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		m_BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		m_BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		m_BlendDesc.RenderTarget[0].LogicOp		= D3D12_LOGIC_OP_NOOP;
		m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	}
} // namespace lde::RHI
