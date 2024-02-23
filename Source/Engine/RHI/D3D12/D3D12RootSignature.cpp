#include <AgilitySDK/d3d12.h>
#include <AgilitySDK/d3dx12/d3dx12.h>
#include "D3D12RootSignature.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	D3D12RootSignature::D3D12RootSignature(D3D12Device* pDevice,
		const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters,
		const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers,
		const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags,
		LPCWSTR DebugName)
	{
		Create(pDevice, Parameters, Samplers, RootFlags, DebugName);
	}
	
	D3D12RootSignature::~D3D12RootSignature()
	{
		Release();
	}
	
	void D3D12RootSignature::Create(D3D12Device* pDevice,
		const std::span<CD3DX12_ROOT_PARAMETER1>& Parameters,
		const std::span<D3D12_STATIC_SAMPLER_DESC>& Samplers,
		const D3D12_ROOT_SIGNATURE_FLAGS& RootFlags,
		LPCWSTR DebugName)
	{
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
		rootDesc.Init_1_1(
			static_cast<uint32>(Parameters.size()), Parameters.data(),
			static_cast<uint32>(Samplers.size()), Samplers.data(),
			RootFlags);
	
		Serialize(pDevice, rootDesc, DebugName);
	}
	
	void D3D12RootSignature::Release()
	{
		SAFE_RELEASE(m_RootSignature);
	}
	
	void D3D12RootSignature::Serialize(D3D12Device* pDevice, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, LPCWSTR DebugName)
	{
		Ref<ID3DBlob> signature;
		Ref<ID3DBlob> error;
	
		DX_CALL(D3DX12SerializeVersionedRootSignature(
			&Desc,
			D3D_ROOT_SIGNATURE_VERSION_1_1,
			&signature, &error));
	
		DX_CALL(pDevice->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&m_RootSignature)));
	
		if (DebugName)
			m_RootSignature.Get()->SetName(DebugName);
	
		SAFE_RELEASE(signature);
		SAFE_RELEASE(error);
	}
} // namespace lde::RHI
