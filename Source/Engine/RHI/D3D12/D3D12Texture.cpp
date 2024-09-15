#include "D3D12Texture.hpp"
#include "D3D12RHI.hpp"
#include "D3D12Utility.hpp"
#include "RHI/RHICommon.hpp"

namespace lde
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
		D3D12Texture::Release();
	}

	void D3D12RenderTexture::Initialize(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName)
	{
		if (Texture.Get())
		{
			SAFE_RELEASE(Texture);
		}

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = ClearColor.at(0);
		clearValue.Color[1] = ClearColor.at(1);
		clearValue.Color[2] = ClearColor.at(2);
		clearValue.Color[3] = ClearColor.at(3);
		//clearValue.Color[3] = 1.0f;
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
			IID_PPV_ARGS(&Texture)));

		if (DebugName.empty())
		{
			Texture->SetName(L"Light Pass Render Texture");
		}
		else
		{
			Texture->SetName(String::ToWide(DebugName).c_str());
		}

		pGfx->Device->CreateRTV(Texture.Get(), GetRTV(), Format);
		pGfx->Device->CreateSRV(Texture.Get(), GetSRV(), 1, 1);

		m_Format	= Format;
		m_Gfx		= pGfx;

	}

	void D3D12RenderTexture::InitializeDepth(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName)
	{
		if (Texture.Get())
		{
			SAFE_RELEASE(Texture);
		}

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = ClearColor.at(0);
		clearValue.Color[1] = ClearColor.at(1);
		clearValue.Color[2] = ClearColor.at(2);
		clearValue.Color[3] = 1.0f;
		clearValue.Format   = Format;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension		= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format			= Format;
		desc.Flags			= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.Width			= static_cast<uint64>(pGfx->SceneViewport->GetViewport().Width);
		desc.Height			= static_cast<uint32>(pGfx->SceneViewport->GetViewport().Height);
		desc.MipLevels		= 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc		= { 1, 0 };

		DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(&Texture)));

		Texture->SetName(String::ToWide(DebugName).c_str());

		pGfx->Device->CreateRTV(Texture.Get(), GetRTV(), Format);
		pGfx->Device->CreateSRV(Texture.Get(), GetSRV(), 1, 1);

		m_Format = Format;
		m_Gfx = pGfx;
	}

	void D3D12RenderTexture::OnResize(uint32 , uint32 )
	{
		if (Texture.Get())
		{
			SAFE_RELEASE(Texture);
		}

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Color[0] = ClearColor.at(0);
		clearValue.Color[1] = ClearColor.at(1);
		clearValue.Color[2] = ClearColor.at(2);
		clearValue.Color[3] = 1.0f;
		clearValue.Format = m_Format;

		D3D12_RESOURCE_DESC desc{};
		desc.Format		= m_Format;
		desc.Dimension	= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.Width		= static_cast<uint64>(m_Gfx->SceneViewport->GetViewport().Width);
		desc.Height		= static_cast<uint32>(m_Gfx->SceneViewport->GetViewport().Height);
		desc.MipLevels	= 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };

		DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&clearValue,
			IID_PPV_ARGS(Texture.GetAddressOf())));

		Texture->SetName(L"Light Pass Render Texture");

		m_Gfx->Device->CreateRTV(Texture.Get(), m_RTV, m_Format);
		m_Gfx->Device->CreateSRV(Texture.Get(), GetSRV(), 1, 1);

	}

} // namespace lde
