#pragma once

namespace lde
{
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
		D3D12Resource(D3D12Device* pDevice)

		ID3D12Resource* GetHandleRaw()
		{
			return m_Resource.Get();
		}

	private:
		Ref<ID3D12Resource> m_Resource;
	};
} // namespace lde
