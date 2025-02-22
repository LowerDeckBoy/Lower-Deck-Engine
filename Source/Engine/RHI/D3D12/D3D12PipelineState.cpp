
#include "D3D12PipelineState.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"

namespace lde
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
	}

	HRESULT D3D12PipelineStateBuilder::Build(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.NodeMask = m_Device->NodeMask;
		desc.pRootSignature = pRootSignature->Get();
		
		// States
		// Rasterizer
		desc.RasterizerState = m_RasterizerDesc;
		desc.RasterizerState.CullMode = m_CullMode;
		desc.RasterizerState.FillMode = m_FillMode;
		// Depth
		desc.DepthStencilState = m_DepthDesc;
		desc.DSVFormat = m_DepthFormat;
		// Blend
		desc.BlendState = m_BlendDesc;

		desc.SampleMask = UINT_MAX;

		auto& shaderCompiler = ShaderCompiler::GetInstance();

		// Compile and set Shaders
		if (!m_VS.Filepath.empty())
		{
			OutPipeline.VertexShader = new Shader(shaderCompiler.Compile(m_VS.Filepath, ShaderStage::eVertex, m_VS.EntryPoint));
			desc.VS = OutPipeline.VertexShader->Bytecode();
		}
		if (!m_PS.Filepath.empty())
		{
			OutPipeline.PixelShader = new Shader(shaderCompiler.Compile(m_PS.Filepath, ShaderStage::ePixel, m_PS.EntryPoint));
			desc.PS = OutPipeline.PixelShader->Bytecode();
		}

		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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
	
	void D3D12PipelineStateBuilder::SetVS(std::string_view Filepath, std::wstring EntryPoint)
	{
		m_VS.Filepath	= Filepath;
		m_VS.EntryPoint	= EntryPoint;
	}
	
	void D3D12PipelineStateBuilder::SetPS(std::string_view Filepath, std::wstring EntryPoint)
	{
		m_PS.Filepath	= Filepath;
		m_PS.EntryPoint = EntryPoint;
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
		m_DepthDesc.DepthEnable				= bEnable;
		m_RasterizerDesc.DepthClipEnable	= bEnable;
	}
	
	void D3D12PipelineStateBuilder::SetRenderTargetFormats(const std::span<DXGI_FORMAT>& Formats)
	{
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.insert(m_RenderTargetFormats.begin(), Formats.begin(), Formats.end());
	}
	
	void D3D12PipelineStateBuilder::Reset()
	{
		m_VS = {};
		m_PS = {};
	
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.shrink_to_fit();
		
		m_RasterizerDesc = {};
		m_FillMode = D3D12_FILL_MODE_SOLID;
		m_CullMode = D3D12_CULL_MODE_NONE;
	
		m_DepthDesc = {};

		// Default Depth Desc
		m_DepthDesc.DepthEnable						= TRUE;
		m_DepthDesc.DepthWriteMask					= D3D12_DEPTH_WRITE_MASK_ALL;
		m_DepthDesc.DepthFunc						= D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_DepthDesc.StencilEnable					= FALSE;
		m_DepthDesc.StencilReadMask					= D3D12_DEFAULT_STENCIL_READ_MASK;
		m_DepthDesc.StencilWriteMask				= D3D12_DEFAULT_STENCIL_WRITE_MASK;
		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		m_DepthDesc.FrontFace = defaultStencilOp;
		m_DepthDesc.BackFace = defaultStencilOp;

		// Default Rasterizer Desc
		m_RasterizerDesc.FillMode					= m_FillMode;
		m_RasterizerDesc.CullMode					= m_CullMode;
		m_RasterizerDesc.FrontCounterClockwise		= FALSE;
		m_RasterizerDesc.DepthBias					= D3D12_DEFAULT_DEPTH_BIAS;
		m_RasterizerDesc.DepthBiasClamp				= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		m_RasterizerDesc.SlopeScaledDepthBias		= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		m_RasterizerDesc.DepthClipEnable			= TRUE;
		m_RasterizerDesc.MultisampleEnable			= FALSE;
		m_RasterizerDesc.AntialiasedLineEnable		= FALSE;
		m_RasterizerDesc.ForcedSampleCount			= 0;
		m_RasterizerDesc.ConservativeRaster			= D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// Default Blend Desc
		m_BlendDesc = {};
		m_BlendDesc.AlphaToCoverageEnable			= FALSE;
		m_BlendDesc.IndependentBlendEnable			= FALSE;
		m_BlendDesc.RenderTarget[0].BlendEnable		= FALSE;
		m_BlendDesc.RenderTarget[0].LogicOpEnable	= FALSE;
		m_BlendDesc.RenderTarget[0].SrcBlend		= D3D12_BLEND_ONE;
		m_BlendDesc.RenderTarget[0].DestBlend		= D3D12_BLEND_ZERO;
		m_BlendDesc.RenderTarget[0].BlendOp			= D3D12_BLEND_OP_ADD;
		m_BlendDesc.RenderTarget[0].SrcBlendAlpha	= D3D12_BLEND_ONE;
		m_BlendDesc.RenderTarget[0].DestBlendAlpha	= D3D12_BLEND_ZERO;
		m_BlendDesc.RenderTarget[0].BlendOpAlpha	= D3D12_BLEND_OP_ADD;
		m_BlendDesc.RenderTarget[0].LogicOp			= D3D12_LOGIC_OP_NOOP;
		m_BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	}
	
	// Mesh
	HRESULT D3D12MeshPipelineBuilder::Build(D3D12Device* pDevice, D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature)
	{
		
		//if (m_AmplificationShader)
		//{
		//	Desc.AS = m_AmplificationShader->Bytecode();
		//}
		if (m_MeshShader)
		{
			Desc.MS = m_MeshShader->Bytecode();
		}
		if (m_PixelShader)
		{
			Desc.PS = m_PixelShader->Bytecode();
		}
		//Desc.AS = nullptr;
		// Those are mendatory
		//assert(m_MeshShader);
		//assert(m_PixelShader);

		Desc.pRootSignature = pRootSignature->Get();

		Desc.NodeMask = pDevice->NodeMask;
		Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		Desc.RasterizerState = m_RasterizerDesc;
		Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		Desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		Desc.DepthStencilState = m_DepthDesc;
		Desc.DSVFormat = m_DepthFormat;

		Desc.NumRenderTargets = 1;
		Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		Desc.SampleMask = UINT_MAX;
		Desc.SampleDesc = { 1, 0 };
		
		//D3D12_PIPELINE_STATE_STREAM_DESC meshStream(&PSODesc);
		auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(Desc);
		
		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
		streamDesc.pPipelineStateSubobjectStream = &psoStream;
		streamDesc.SizeInBytes = sizeof(psoStream);

		return pDevice->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&OutPipeline.PipelineState));
	}

	void D3D12MeshPipelineBuilder::SetAS(std::string_view Filepath, std::wstring EntryPoint)
	{
		m_AmplificationShader = new Shader(ShaderCompiler::GetInstance().Compile(Filepath, ShaderStage::eAmplification, EntryPoint));
	}

	void D3D12MeshPipelineBuilder::SetMS(std::string_view Filepath, std::wstring EntryPoint)
	{
		m_MeshShader = new Shader(ShaderCompiler::GetInstance().Compile(Filepath, ShaderStage::eMesh, EntryPoint));
	}

	void D3D12MeshPipelineBuilder::SetPS(std::string_view Filepath, std::wstring EntryPoint)
	{
		m_PixelShader = new Shader(ShaderCompiler::GetInstance().Compile(Filepath, ShaderStage::ePixel, EntryPoint));
	}


} // namespace lde
