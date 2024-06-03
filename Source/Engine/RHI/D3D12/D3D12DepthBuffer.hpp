#pragma once

namespace lde::RHI
{
	class D3D12Device;
	class D3D12Descriptor;
	class D3D12DescriptorHeap;
	class D3D12Viewport;
	
	class D3D12DepthBuffer
	{
	public:
		D3D12DepthBuffer() = default;
		D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		~D3D12DepthBuffer();

		void Create(D3D12Device* pDevice, D3D12Viewport* pViewport);

		void OnResize(D3D12Viewport* pViewport);

		ID3D12Resource* Get() { return m_Resource.Get(); }

		inline D3D12Descriptor DSV() const { return m_DSV; }
		inline D3D12Descriptor SRV() const { return m_SRV; }

		void Release();

	private:
		D3D12Device* m_Device = nullptr;
		Ref<ID3D12Resource> m_Resource;
		DXGI_FORMAT m_Format = DXGI_FORMAT_D32_FLOAT;

		D3D12Descriptor m_DSV;
		D3D12Descriptor m_SRV;

	};
} // namespace lde::RHI
