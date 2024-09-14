#pragma once

/*

*/

#include <RHI/Texture.hpp>
#include "D3D12Descriptor.hpp"
#include <Core/CoreMinimal.hpp>

namespace lde
{
	class D3D12RHI;

	struct TextureDesc
	{
		std::string Name = "";
		uint64	Width	= 0;
		uint32	Height	= 0;
		// if Mips are higher than 1, sets texture to UAV
		uint16	Mips	= 1; 
		uint16	Depth	= 1;
		bool	bMSAA	= false;
	};
	
	class D3D12Texture
	{
	public:
		D3D12Texture() = default;
		~D3D12Texture() = default;

		void Create(TextureDesc Desc);
		void Release();

		D3D12Descriptor SRV;
		D3D12Descriptor UAV; // For mipmapping

		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		uint32 Width		= 0; // 0 means invalid
		uint32 Height		= 0; // 0 means invalid
		uint16 MipLevels	= 1; // default; no mipmaps

		Ref<ID3D12Resource> Texture;
	protected:


	};

	/**
	 * @brief Texture is created in Generic Read state
	 */
	class D3D12RenderTexture : public D3D12Texture
	{
	public:
		D3D12RenderTexture() = default;
		D3D12RenderTexture(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName = "");
		~D3D12RenderTexture(); /* Releases Texture resource */

		void Initialize(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName = "");
		void InitializeDepth(D3D12RHI* pGfx, DXGI_FORMAT Format, std::string_view DebugName = "");

		void OnResize(uint32 Width, uint32 Height);

		D3D12Descriptor& GetSRV() { return SRV; }
		D3D12Descriptor& GetRTV() { return m_RTV; }
		D3D12Descriptor& GetDSV() { return m_DSV; }

		DXGI_FORMAT& GetFormat() { return m_Format; }

		/**
		 * @brief 
		 * @return Underlying texture resource.
		 */
		ID3D12Resource* Get() { return Texture.Get(); }

	private:
		// For texture resizing
		D3D12RHI* m_Gfx = nullptr;
		//Ref<ID3D12Resource> m_Texture;
		//DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		//D3D12Descriptor m_SRV;
		D3D12Descriptor m_RTV;
		D3D12Descriptor m_DSV; // Shadow maps

	};

} // namespace lde
