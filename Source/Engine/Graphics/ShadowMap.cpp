#include "RHI/D3D12/D3D12Viewport.hpp"
#include "RHI/D3D12/D3D12Device.hpp"
#include "ShadowMap.hpp"
#include "Platform/Window.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"

namespace lde
{
	ShadowMap::ShadowMap(D3D12Device* /* pDevice */)
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = static_cast<uint64>(Window::Width);
		desc.Height = Window::Height;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = D3D12_MAX_DEPTH;
		clearValue.DepthStencil.Stencil = 0;

		//RHI::DX_CALL(pDevice->GetDevice()->CreateCommittedResource(
		//	&RHI::D3D12Utility::HeapDefault,
		//	D3D12_HEAP_FLAG_NONE,
		//	&desc,
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	&clearValue,
		//	IID_PPV_ARGS(&m_Resource)
		//));
		
	}

	ShadowMap::~ShadowMap()
	{
	}

	void ShadowMap::Render(DirectX::XMFLOAT3 Position)
	{
		m_View = DirectX::XMMatrixLookAtLH(XMLoadFloat3(&Position), m_Target, m_Up);
		m_Projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), 1.0f, 0.1f, 50.0f);

		//DirectX::XMMATRIX T(
		//	0.5f, 0.0f, 0.0f, 0.0f,
		//	0.0f, -0.5f, 0.0f, 0.0f,
		//	0.0f, 0.0f, 1.0f, 0.0f,
		//	0.5f, 0.5f, 0.0f, 1.0f);

		//Pre-transpose
		DirectX::XMMATRIX T2(
			0.5f, 0.0f, 0.0f, 0.5f,
			0.0f, 0.5f, 0.0f, 0.5f,
			0.0f, 0.0f, 0.5f, 0.5f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		//  
		DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity() * m_View * m_Projection) * T2;

	}
}
