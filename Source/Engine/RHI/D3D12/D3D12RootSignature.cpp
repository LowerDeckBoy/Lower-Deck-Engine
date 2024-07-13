#include <AgilitySDK/d3d12.h>
#include "D3D12RootSignature.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	D3D12RootSignature::~D3D12RootSignature()
	{
		Release();
	}

	void D3D12RootSignature::AddConstants(uint32 Count, uint32 RegisterSlot, uint32 Space, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		parameter.Constants.Num32BitValues	= Count;
		parameter.Constants.ShaderRegister	= RegisterSlot;
		parameter.Constants.RegisterSpace	= Space;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddCBV(uint32 RegisterSlot, uint32 Space, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameter.Descriptor.ShaderRegister = RegisterSlot;
		parameter.Descriptor.RegisterSpace	= Space;
		parameter.Descriptor.Flags			= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddSRV(uint32 RegisterSlot, uint32 Space, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_SRV;
		parameter.Descriptor.ShaderRegister = RegisterSlot;
		parameter.Descriptor.RegisterSpace	= Space;
		parameter.Descriptor.Flags			= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddUAV(uint32 RegisterSlot, uint32 Space, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType				= D3D12_ROOT_PARAMETER_TYPE_UAV;
		parameter.Descriptor.ShaderRegister = RegisterSlot;
		parameter.Descriptor.RegisterSpace	= Space;
		parameter.Descriptor.Flags			= D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
		parameter.ShaderVisibility			= Visibility;

		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddDescriptorTable(std::span<D3D12_DESCRIPTOR_RANGE1> Ranges, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_ROOT_PARAMETER1 parameter{};
		parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter.DescriptorTable.NumDescriptorRanges = static_cast<uint32>(Ranges.size());
		parameter.DescriptorTable.pDescriptorRanges = Ranges.data();
		parameter.ShaderVisibility = Visibility;
		
		m_Parameters.emplace_back(parameter);
	}

	void D3D12RootSignature::AddStaticSampler(uint32 RegisterSlot, uint32 Space, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc)
	{
		D3D12_STATIC_SAMPLER_DESC1 sampler{};
		sampler.ShaderRegister	= RegisterSlot;
		sampler.RegisterSpace	= Space;
		sampler.Filter			= Filter;
		sampler.AddressU		= AddressMode;
		sampler.AddressV		= AddressMode;
		sampler.AddressW		= AddressMode;
		sampler.ComparisonFunc	= ComparsionFunc;
		sampler.BorderColor		= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD			= 0.0f;
		sampler.MaxLOD			= D3D12_FLOAT32_MAX;
		sampler.MaxAnisotropy	= D3D12_MAX_MAXANISOTROPY;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampler.Flags			= D3D12_SAMPLER_FLAG_NONE;

		m_StaticSamplers.emplace_back(sampler);
	}

	void D3D12RootSignature::SetLocal()
	{
		bIsLocal = true;
	}

	HRESULT D3D12RootSignature::Build(D3D12Device* pDevice, PipelineType eType, std::string DebugName)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc{};
		desc.Version					= D3D_ROOT_SIGNATURE_VERSION_1_2;
		desc.Desc_1_2.NumParameters		= static_cast<uint32>(m_Parameters.size());
		desc.Desc_1_2.pParameters		= m_Parameters.data();
		desc.Desc_1_2.NumStaticSamplers = static_cast<uint32>(m_StaticSamplers.size());
		desc.Desc_1_2.pStaticSamplers	= m_StaticSamplers.data();
		desc.Desc_1_2.Flags				= (bIsLocal) ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : m_RootFlags;

		Type = eType;

		ID3DBlob* signature = nullptr;
		ID3DBlob* error = nullptr;

		DX_CALL(D3D12SerializeVersionedRootSignature(&desc, &signature, &error), "Failed to serialize RootSignature!");
		
		if (error)
		{
			std::string message = std::format("Root Signature building error:\n, {}", (char*)error->GetBufferPointer());
			::MessageBoxA(nullptr, message.c_str(), "D3D12 Error", MB_OK);
			throw std::exception();
		}

		HRESULT result = pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		DX_CALL(result);

		if (!DebugName.empty())
		{
			m_RootSignature->SetName(String::ToWide(DebugName).c_str());
		}
		
		return result;
	}

	void D3D12RootSignature::Release()
	{
		SAFE_RELEASE(m_RootSignature);
	}
	
} // namespace lde::RHI
