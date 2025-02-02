#pragma once

#include <AgilitySDK/d3d12.h>

namespace lde
{
	class D3D12Device;

	enum class ResourceType
	{
		eBuffer,
		eTexture
	};

	struct ResourceDesc
	{

	};

	class D3D12Resource
	{
	public:
		D3D12Resource(D3D12Device* pDevice);
		~D3D12Resource();

		void Release();

		void SetDebugName(std::string_view Name);

	private:
		Ref<ID3D12Resource>			m_ResourceHandle;
		Ref<D3D12MA::Allocation>	m_ResourceAllocation;

	};
} // namespace lde
