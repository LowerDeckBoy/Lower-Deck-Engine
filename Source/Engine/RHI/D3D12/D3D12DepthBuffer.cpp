#include "D3D12Device.hpp"
#include "D3D12Descriptor.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12Viewport.hpp"
#include "D3D12DepthBuffer.hpp"
#include "D3D12Utility.hpp"
#include <AgilitySDK/d3dx12/d3dx12.h>

namespace lde::RHI
{
	D3D12DepthBuffer::D3D12DepthBuffer(D3D12Device* pDevice, D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, DXGI_FORMAT Format, bool bSRV)
	{
		m_Format = Format;
		m_Device = pDevice;
		Create(pDevice, pDepthHeap, pViewport, bSRV);
	}

	D3D12DepthBuffer::~D3D12DepthBuffer()
	{
		Release();
	}

	void D3D12DepthBuffer::Create(D3D12Device* pDevice, D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport, bool bSRV)
	{
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };

		D3D12_RESOURCE_DESC desc{};
		desc.Format = m_Format;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Width  = static_cast<uint64>(pViewport->GetViewport().Width);
		desc.Height = static_cast<uint32>(pViewport->GetViewport().Height);
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.SampleDesc = { 1, 0 };

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

		m_DSV = pDepthHeap->Allocate();
		pDevice->GetDevice()->CreateDepthStencilView(m_Resource.Get(), &view, m_DSV.GetCpuHandle());

		if (bSRV)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (m_Format == DXGI_FORMAT_D32_FLOAT)
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			else if (m_Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
				srvDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

			//D3D12Context::GetMainHeap()->Allocate(m_SRV);
			//g_Device.Get()->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_SRV.GetCPU());
		}

	}

	void D3D12DepthBuffer::Clear()
	{
	}

	void D3D12DepthBuffer::OnResize(D3D12DescriptorHeap* pDepthHeap, D3D12Viewport* pViewport)
	{
		if (m_Resource.Get())
		{
			SAFE_RELEASE(m_Resource);
			//m_Resource.Reset();
		}

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_Format;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		const auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
		const auto heapDesc{ CD3DX12_RESOURCE_DESC::Tex2D(m_Format,
													static_cast<uint64_t>(pViewport->GetViewport().Width),
													static_cast<uint32_t>(pViewport->GetViewport().Height),
													1, 0, 1, 0,
													D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) };

		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&heapDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_Resource)
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsView{};
		dsView.Flags = D3D12_DSV_FLAG_NONE;
		dsView.Format = m_Format;
		dsView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsView.Texture2D.MipSlice = 0;

		m_DSV = pDepthHeap->Allocate();
		m_Device->GetDevice()->CreateDepthStencilView(m_Resource.Get(), &dsView, m_DSV.GetCpuHandle());

	}

	void D3D12DepthBuffer::Release()
	{
		SAFE_RELEASE(m_Resource);
	}
}
