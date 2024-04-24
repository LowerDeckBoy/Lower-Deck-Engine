#include "D3D12Texture.hpp"
#include "D3D12RHI.hpp"
#include "D3D12Utility.hpp"
#include "RHI/RHICommon.hpp"

namespace lde::RHI
{
	void D3D12Texture::Release()
	{
		SAFE_RELEASE(Texture);
	}

	/* ========================== Render Texture ========================== */

	D3D12RenderTexture::D3D12RenderTexture(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName)
	{
		Initialize(pGfx, Format, DebugName);
	}

	D3D12RenderTexture::~D3D12RenderTexture()
	{
		SAFE_RELEASE(m_Texture);
	}

	void D3D12RenderTexture::Initialize(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName)
	{
		if (m_Texture.Get())
			m_Texture.Reset();

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = ClearColor.at(0);
		clearValue.Color[1] = ClearColor.at(1);
		clearValue.Color[2] = ClearColor.at(2);
		clearValue.Color[3] = 1.0f;
		clearValue.Format   = Format;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension	= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format		= Format;
		desc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.Width		= static_cast<uint64>(pGfx->SceneViewport->GetViewport().Width);
		desc.Height		= static_cast<uint32>(pGfx->SceneViewport->GetViewport().Height);
		desc.MipLevels	= 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };

		DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(&m_Texture)));

		m_Texture->SetName(String::ToWide(DebugName).c_str());

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		pGfx->Device->GetRTVHeap()->Allocate(GetRTV());
		pGfx->Device->GetDevice()->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, GetRTV().GetCpuHandle());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = Format;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		((D3D12Device*)pGfx->GetDevice())->Allocate(HeapType::eSRV, GetSRV(), 1);
		pGfx->Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, GetSRV().GetCpuHandle());

		m_Format = Format;
		m_Gfx = pGfx;

	}

	void D3D12RenderTexture::OnResize(uint32 , uint32 )
	{
		if (m_Texture.Get())
			m_Texture.Reset();

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = ClearColor.at(0);
		clearValue.Color[1] = ClearColor.at(1);
		clearValue.Color[2] = ClearColor.at(2);
		clearValue.Color[3] = 1.0f;
		clearValue.Format = m_Format;

		D3D12_RESOURCE_DESC desc{};
		desc.Format = m_Format;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.Width  = static_cast<uint64>(m_Gfx->SceneViewport->GetViewport().Width);
		desc.Height = static_cast<uint32>(m_Gfx->SceneViewport->GetViewport().Height);
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };

		DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(&m_Texture)));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = m_Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		m_Gfx->Device->GetRTVHeap()->Allocate(GetRTV());
		m_Gfx->Device->GetDevice()->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, GetRTV().GetCpuHandle());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		((D3D12Device*)m_Gfx->GetDevice())->Allocate(HeapType::eSRV, GetSRV(), 1);
		m_Gfx->Device->GetDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, GetSRV().GetCpuHandle());
	}
}
