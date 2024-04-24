#pragma once

#include <RHI/D3D12/D3D12Texture.hpp>

namespace lde::RHI
{
	class D3D12Device;
}

namespace lde
{ 
	class ShadowMap
	{
	public:
		ShadowMap(RHI::D3D12Device* pDevice);

		void Render();
		void Resize(uint32 Width, uint32 Height);
		
	private:
		RHI::D3D12RenderTexture m_RenderTexture;

		Ref<ID3D12Resource> m_Resource;

	};
} // namespace lde
