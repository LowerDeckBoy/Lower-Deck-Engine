#pragma once

/*

*/


#include <Core/CoreMinimal.hpp>
#include <RHI/Types.hpp>
#include <span>

namespace lde::RHI
{
	class D3D12Device;
	
	class D3D12RootSignature
	{
	public:
		D3D12RootSignature() = default;
		D3D12RootSignature(D3D12Device* pDevice,
			const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters,
			const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers,
			const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags,
			LPCWSTR DebugName = L"");
		~D3D12RootSignature();
	
		void Create(D3D12Device* pDevice,
			const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters,
			const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers,
			const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags,
			LPCWSTR DebugName = L"");
	
		inline ID3D12RootSignature* GetRootSignature()
		{
			return m_RootSignature.Get();
		}
	
		PipelineType Type{};
	
		void Release();
	
	private:
		Ref<ID3D12RootSignature> m_RootSignature;
	
		void Serialize(D3D12Device* pDevice, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, LPCWSTR DebugName);
	
	};
} // namespace lde::RHI
