#pragma once

#include "RHI/D3D12/D3D12Texture.hpp"

namespace lde::RHI
{
	class D3D12Device;
	//class D3D12Viewport;
}

namespace lde
{ 
	class ShadowMap
	{
	public:
		ShadowMap(D3D12Device* pDevice);
		~ShadowMap();

		void Render(DirectX::XMFLOAT3 Position);
		void Resize(uint32 Width, uint32 Height);
		
	private:
		D3D12RenderTexture m_RenderTexture;
		//D3D12Viewport		m_Viewport;
		//Ref<ID3D12Resource>		m_Resource;

		DirectX::XMVECTOR m_Target		= DirectX::XMVECTOR();
		DirectX::XMVECTOR m_Up			= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX m_View		= DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_Projection	= DirectX::XMMatrixIdentity();

	};
} // namespace lde
