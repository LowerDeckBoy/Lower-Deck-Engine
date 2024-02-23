#pragma once

/*

*/

#include "D3D12Descriptor.hpp"
#include <Core/CoreMinimal.hpp>

namespace lde::RHI
{
	class D3D12Context;

	/// @brief 
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

	/// @brief 
	class D3D12RenderTexture
	{
	public:
		~D3D12RenderTexture(); /* Releases Texture resource */

		void Initialize(D3D12Context* pGfx, DXGI_FORMAT Format, LPCWSTR DebugName = L"");

		void OnResize(uint32 Width, uint32 Height);

		D3D12Descriptor SRV;
		D3D12Descriptor RTV;

		DXGI_FORMAT& GetFormat() { return m_Format; }

		Ref<ID3D12Resource> Resource;

	private:
		// For texture resizing
		D3D12Context* m_Gfx = nullptr;
		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	};

} // namespace lde::RHI
