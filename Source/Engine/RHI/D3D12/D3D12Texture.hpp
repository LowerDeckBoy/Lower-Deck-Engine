#pragma once

/*

*/

#include <RHI/Texture.hpp>
#include "D3D12Descriptor.hpp"
#include <Core/CoreMinimal.hpp>

namespace lde::RHI
{
	class D3D12RHI;

	
	class D3D12Texture
	{
	public:
		~D3D12Texture(); /* Releases Texture resource */

		D3D12Descriptor SRV;
		D3D12Descriptor UAV; /* For mipmapping */

		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		uint32 Width		= 0; // 0 means invalid
		uint32 Height		= 0; // 0 means invalid
		uint16 MipLevels	= 1; // default; no mipmaps

		Ref<ID3D12Resource> Texture;
	private:


	};

	
	class D3D12RenderTexture
	{
	public:
		~D3D12RenderTexture(); /* Releases Texture resource */

		void Initialize(D3D12RHI* pGfx, DXGI_FORMAT Format, LPCWSTR DebugName = L"");

		void OnResize(uint32 Width, uint32 Height);

		D3D12Descriptor& GetSRV() { return m_SRV; }
		D3D12Descriptor& GetRTV() { return m_RTV; }

		DXGI_FORMAT& GetFormat() { return m_Format; }

		Ref<ID3D12Resource> Resource;

	private:
		// For texture resizing
		D3D12RHI* m_Gfx = nullptr;
		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		D3D12Descriptor m_SRV;
		D3D12Descriptor m_RTV;
	};

} // namespace lde::RHI
