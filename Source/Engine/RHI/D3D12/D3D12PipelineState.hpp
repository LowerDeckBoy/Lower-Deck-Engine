#pragma once

/*
	
*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <Graphics/ShaderCompiler.hpp>
#include <RHI/Types.hpp>
#include <span>
#include <vector>


namespace lde::RHI
{
	class D3D12Device;
	class D3D12RootSignature;

	// TODO: Move RootSignature here, so PSO holds specific RS

	/// @brief Hold PSO, type of pipeline and shaders.
	struct D3D12PipelineState
	{
		~D3D12PipelineState();
	
		inline ID3D12PipelineState* Get() const
		{
			return PipelineState.Get();
		}
	
		Ref<ID3D12PipelineState> PipelineState;
		D3D12RootSignature* RootSignature = nullptr;
		PipelineType Type{};

		Shader VertexShader;
		Shader PixelShader;
		Shader DomainShader;
		Shader HullShader;
		Shader GeometryShader;
	};

	// Graphics Pipeline State builder class.
	class D3D12PipelineStateBuilder
	{
	public:
		D3D12PipelineStateBuilder(D3D12Device* pDevice);
		~D3D12PipelineStateBuilder();
	
		HRESULT Build(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature, const std::string& DebugName = "");

		HRESULT BuildMesh(D3D12PipelineState& OutPipeline, D3D12RootSignature* pRootSignature, const std::string& DebugName = "");
	
		void SetVS(std::string_view Filepath, std::wstring EntryPoint = L"main");
		void SetPS(std::string_view Filepath, std::wstring EntryPoint = L"main");
		void SetGS(std::string_view Filepath, std::wstring EntryPoint = L"main");
		void SetHS(std::string_view Filepath, std::wstring EntryPoint = L"main");
		void SetTS(std::string_view Filepath, std::wstring EntryPoint = L"main");
		void SetDS(std::string_view Filepath, std::wstring EntryPoint = L"main");
	
		void SetCullMode(CullMode eMode);
	
		void SetWireframe(bool bEnable);
	
		void SetDepthFormat(DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
	
		void EnableDepth(bool bEnable);
	
		void SetRenderTargetFormats(const std::span<DXGI_FORMAT>& Formats);
	
		void Reset();
	
	private:
		D3D12Device* m_Device = nullptr;
	
		// Number of Render Targets is determine by size of vector.
		std::vector<DXGI_FORMAT> m_RenderTargetFormats;

		Shader* m_AmplificationShader	= nullptr;
		Shader* m_MeshShader			= nullptr;

		/*
			Note: if Mesh shaders are available, other shaders cannot be used!
		*/

		Shader* m_VertexShader		= nullptr;
		Shader* m_PixelShader		= nullptr;
		Shader* m_GeometryShader	= nullptr;
		Shader* m_HullShader		= nullptr;
		Shader* m_TessellationShader= nullptr;
		Shader* m_DomainShader		= nullptr;
		
		D3D12_RASTERIZER_DESC m_RasterizerDesc{};
		D3D12_CULL_MODE m_CullMode = D3D12_CULL_MODE_BACK;
		D3D12_FILL_MODE m_FillMode = D3D12_FILL_MODE_SOLID;

		D3D12_DEPTH_STENCIL_DESC m_DepthDesc{};
		DXGI_FORMAT m_DepthFormat = DXGI_FORMAT_D32_FLOAT;

		D3D12_BLEND_DESC m_BlendDesc{};
	
	};
} // namespace lde::RHI
