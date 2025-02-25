#pragma once

#include <AgilitySDK/d3d12.h>
#include "Core/CoreMinimal.hpp"
#include "RHI/Types.hpp"
#include <span>
#include <vector>

namespace lde
{
	class D3D12Device;
	class Shader;

	class D3D12RootSignature
	{
	public:
		D3D12RootSignature() = default;
		~D3D12RootSignature();
	
		inline ID3D12RootSignature* Get()
		{
			return m_RootSignature.Get();
		}
	
		// Do as much Constants as possible; most efficient.
		void AddConstants(uint32 Count, uint32 RegisterSlot, uint32 Space = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		// If possible, use instead of DescriptorTables.
		void AddCBV(uint32 RegisterSlot, uint32 Space = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		// If possible, use instead of DescriptorTables.
		void AddSRV(uint32 RegisterSlot, uint32 Space = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		// If possible, use instead of DescriptorTables.
		void AddUAV(uint32 RegisterSlot, uint32 Space = 0, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		// Least preferable; slowest. Go for bindless instead.
		void AddDescriptorTable(std::span<D3D12_DESCRIPTOR_RANGE1> Ranges, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);
		
		void AddStaticSampler(uint32 RegisterSlot, uint32 Space, 
			D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, 
			D3D12_COMPARISON_FUNC ComparsionFunc = D3D12_COMPARISON_FUNC_ALWAYS);

		/**
		 * @brief Note: when adding Paramters and StaticSamplers they are being pushed to the vector;
		 * order of Parameters is from top-to-bottom.
		 * 
		 * @param pDevice		Parent Device
		 * @param eType			Whether RootSignature is for Graphics or Compute state.
		 * @param DebugName		[Optional]
		 * @return 
		 */
		HRESULT Build(D3D12Device* pDevice, PipelineType eType, std::string DebugName = "");

		// TODO:
		// Build RootSignature based on given Shader
		// Note: if building this way - don't set neither Paramters nor StaticSamplers
		void Build(D3D12Device* pDevice, PipelineType eType, Shader* pShader, std::string DebugName = "");

		void Release();
	
		PipelineType Type{};
	private:
		Ref<ID3D12RootSignature> m_RootSignature;

		std::vector<D3D12_ROOT_PARAMETER1>		m_Parameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC1> m_StaticSamplers;
		
		D3D12_ROOT_SIGNATURE_FLAGS m_RootFlags = 
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		
	};
} // namespace lde
