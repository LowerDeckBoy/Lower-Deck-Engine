#include "D3D12PipelineState.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{

	D3D12PipelineState::~D3D12PipelineState()
	{
		SAFE_RELEASE(PipelineState);
	}
	
	D3D12PipelineStateBuilder::D3D12PipelineStateBuilder(D3D12Device* pDevice)
		: m_Device(pDevice)
	{
	}
	
	D3D12PipelineStateBuilder::~D3D12PipelineStateBuilder()
	{
		Reset();
	}

	HRESULT D3D12PipelineStateBuilder::Build(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature, const std::string& DebugName)
	{
		//D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{};
		////streamDesc.pPipelineStateSubobjectStream
		//DX_CALL(m_Device->Device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&OutPipeline.PipelineState)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.NodeMask = 0;
		desc.pRootSignature = pRootSignature->GetRootSignature();

		// Not required for bindless
		//desc.InputLayout = { m_InputLayout.data(), static_cast<uint32>(m_InputLayout.size()) };
	 
		// States
		// Rasterizer
		desc.RasterizerState = m_RasterizerDesc;
		desc.RasterizerState.CullMode = m_CullMode;
		desc.RasterizerState.FillMode = m_FillMode;
		desc.RasterizerState.MultisampleEnable = true;
		// Depth
		desc.DepthStencilState = m_DepthDesc;
		desc.DSVFormat = m_DepthFormat;
		// Blend
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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

		// Render Targets
		if (!m_RenderTargetFormats.empty())
		{
			desc.NumRenderTargets = static_cast<uint32>(m_RenderTargetFormats.size());
			for (uint32 i = 0; i < static_cast<uint32>(m_RenderTargetFormats.size()); i++)
				desc.RTVFormats[i] = m_RenderTargetFormats.at(i);
		}
		else
		{
			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		OutPipeline.Type = PipelineType::eGraphics;
		OutPipeline.RootSignature = pRootSignature;
		return m_Device->GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&OutPipeline.PipelineState));

	}

	void D3D12PipelineStateBuilder::SetVertexShader(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderManager = Singleton<ShaderManager>::GetInstance();
		if (m_VertexShader) m_VertexShader = nullptr;
		m_VertexShader = new Shader(shaderManager.Compile(Filepath, ShaderStage::eVertex, EntryPoint));
	}
	
	void D3D12PipelineStateBuilder::SetPixelShader(std::string_view Filepath, std::wstring EntryPoint)
	{
		auto& shaderManager = Singleton<ShaderManager>::GetInstance();
		if (m_PixelShader) m_PixelShader = nullptr;
		m_PixelShader = new Shader(shaderManager.Compile(Filepath, ShaderStage::ePixel, EntryPoint));
	}

	void D3D12PipelineStateBuilder::SetInputLayout(const std::span<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
	{
		m_InputLayout.clear();
		m_InputLayout.insert(m_InputLayout.begin(), InputLayout.begin(), InputLayout.end());
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
		m_PixelShader = nullptr;
	
		m_RenderTargetFormats.clear();
		m_RenderTargetFormats.shrink_to_fit();
	
		m_Ranges.clear();
		m_Parameters.clear();
		//m_InputLayout = {};
		
		m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_FillMode = D3D12_FILL_MODE_SOLID;
		m_CullMode = D3D12_CULL_MODE_BACK;
	
		m_DepthDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	}
} // namespace lde::RHI
