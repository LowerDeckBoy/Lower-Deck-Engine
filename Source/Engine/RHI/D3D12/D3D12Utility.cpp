#include "D3D12Utility.hpp"

#if defined _DEBUG
#	include <Windows.h>
#	include <debugapi.h>
#	include <comdef.h>
#endif

namespace lde::RHI
{
	void VerifyResult(HRESULT hResult, const char* File, int Line, std::string_view Message)
	{
		if (SUCCEEDED(hResult))
			return;

		auto comError = _com_error(hResult).ErrorMessage();
		std::wstring wstr = std::wstring(comError);
		auto out = String::WCharToChar(wstr.data());
		std::string message = "";
		if (!Message.empty())
		{
			message = std::format("{}\n\n{}\nFile: {}\nLine: {}", Message, out, File, Line);
		}
		else
		{
			message = std::format("{}\n\nFile: {}\nLine: {}", out, File, Line);
		}

		::MessageBoxA(nullptr, message.c_str(), "D3D12 Error", MB_OK);

		throw std::exception();
	}

	void SetName(ID3D12Object* pDxObject, std::string Name)
	{
		pDxObject->SetName(String::ToWide(Name).c_str());
	}

	void D3D12Utility::CreateUAV(ID3D12Device8* pDevice, ID3D12Resource** ppTargetResource, size_t BufferSize, D3D12_RESOURCE_STATES InitialState)
	{
		const D3D12_HEAP_PROPERTIES uploadHeapProperties = HeapDefault;
		D3D12_RESOURCE_DESC desc{};
		desc.Width = static_cast<uint64_t>(BufferSize);
		desc.Height = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc = { 1, 0 };

		DX_CALL(pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			InitialState,
			nullptr,
			IID_PPV_ARGS(ppTargetResource)));
	}

	Ref<ID3D12Resource> D3D12Utility::CreateUAVBuffer(ID3D12Device8* pDevice, uint64_t Size, D3D12_RESOURCE_FLAGS Flags, D3D12_RESOURCE_STATES InitState, D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags)
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Flags = Flags;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Height = 1;
		desc.Width = Size;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc = { 1, 0 };

		Ref<ID3D12Resource> output;
		DX_CALL(pDevice->CreateCommittedResource(&HeapProps, HeapFlags, &desc, InitState, nullptr, IID_PPV_ARGS(&output)));

		return output;
	}

	void D3D12Utility::CreateUAVBuffer(ID3D12Device8* pDevice, Ref<ID3D12Resource>& Target, uint64_t Size, D3D12_RESOURCE_FLAGS Flags, D3D12_RESOURCE_STATES InitState, D3D12_HEAP_PROPERTIES& HeapProps, D3D12_HEAP_FLAGS HeapFlags)
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Flags = Flags;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Height = 1;
		desc.Width = Size;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc = { 1, 0 };

		DX_CALL(pDevice->CreateCommittedResource(&HeapProps, HeapFlags, &desc, InitState, nullptr, IID_PPV_ARGS(Target.ReleaseAndGetAddressOf())));
	}

	D3D12_STATIC_SAMPLER_DESC D3D12Utility::CreateStaticSampler(uint32_t ShaderRegister, uint32_t RegisterSpace, D3D12_FILTER Filter, D3D12_TEXTURE_ADDRESS_MODE AddressMode, D3D12_COMPARISON_FUNC ComparsionFunc, D3D12_SHADER_VISIBILITY Visibility)
	{
		D3D12_STATIC_SAMPLER_DESC sampler{};
		sampler.AddressU = AddressMode;
		sampler.AddressV = AddressMode;
		sampler.AddressW = AddressMode;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.ComparisonFunc = ComparsionFunc;
		sampler.Filter = Filter;
		sampler.MaxAnisotropy = 0;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = static_cast<float>(UINT32_MAX);
		sampler.ShaderRegister = ShaderRegister;
		sampler.RegisterSpace = RegisterSpace;
		sampler.ShaderVisibility = Visibility;

		return sampler;
	}
}
