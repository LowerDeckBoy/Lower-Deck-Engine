#include "D3D12Device.hpp"
#include "D3D12DepthBuffer.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Viewport.hpp"

namespace lde
{
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format)
	{
		m_Format = Format;
		m_Device = pDevice;
		Create(pDevice, pViewport);
	}

	D3D12DepthBuffer::~D3D12DepthBuffer()
	{
		Release();
	}

	void D3D12DepthBuffer::Create(D3D12Device* pDevice, D3D12Viewport* pViewport)
	{
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;
		
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
			&D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_Resource)
		));
		SET_D3D12_NAME(m_Resource, "D3D12 Depth Buffer");

		pDevice->CreateDSV(m_Resource.Get(), m_DSV);
	}
	
	void D3D12DepthBuffer::OnResize(D3D12Viewport* pViewport)
	{
		if (m_Resource.Get())
		{
			SAFE_RELEASE(m_Resource);
		}

		Create(m_Device, pViewport);
	}

	void D3D12DepthBuffer::Release()
	{
		SAFE_RELEASE(m_Resource);
	}
}
