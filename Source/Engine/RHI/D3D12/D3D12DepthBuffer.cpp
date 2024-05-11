#include "D3D12Device.hpp"
#include "D3D12DepthBuffer.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Viewport.hpp"

namespace lde::RHI
{
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12Device* pDevice, D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, DXGI_FORMAT Format)
	{
		m_Format = Format;
		m_Device = pDevice;
		Create(pDevice, pDepthHeap, pViewport);
	}

	D3D12DepthBuffer::~D3D12DepthBuffer()
	{
		Release();
	}

	void D3D12DepthBuffer::Create(D3D12Device* pDevice, D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport)
	{
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProperties = D3D12Utility::HeapDefault;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width				= static_cast<uint64>(pViewport->GetViewport().Width);
		desc.Height				= static_cast<uint32>(pViewport->GetViewport().Height);
		desc.Format				= m_Format;
		desc.MipLevels			= 1;
		desc.DepthOrArraySize	= 1;
		desc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		desc.SampleDesc			= { 1, 0 };

		DX_CALL(pDevice->GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_Resource)
		));
		m_Resource->SetName(L"Depth Buffer");

		D3D12_DEPTH_STENCIL_VIEW_DESC view{};
		view.Flags = D3D12_DSV_FLAG_NONE;
		view.Format = m_Format;
		view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;

		pDepthHeap->Allocate(m_DSV);
		pDevice->GetDevice()->CreateDepthStencilView(m_Resource.Get(), &view, m_DSV.GetCpuHandle());
		
	}
	
	void D3D12DepthBuffer::OnResize(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport)
	{
		if (m_Resource.Get())
		{
			SAFE_RELEASE(m_Resource);
		}

		Create(m_Device, pDepthHeap, pViewport);
	}

	void D3D12DepthBuffer::Release()
	{
		SAFE_RELEASE(m_Resource);
	}
}
