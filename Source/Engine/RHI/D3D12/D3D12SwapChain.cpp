#include "D3D12DescriptorHeap.hpp"
#include "D3D12Device.hpp"
#include "D3D12Queue.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Utility.hpp"
#include "Platform/Window.hpp"

namespace lde
{
	D3D12SwapChain::D3D12SwapChain(D3D12Device* pDevice, D3D12Queue* pQueue, uint32 Width, uint32 Height)
	{
		m_Device = pDevice;

		m_DescriptorHeap = std::make_unique<D3D12DescriptorHeap>(pDevice, HeapType::eRTV, FRAME_COUNT, "SwapChain RTV Heap");

		Initialize(pDevice, pQueue, Width, Height);
	}

	D3D12SwapChain::~D3D12SwapChain()
	{
		Release();
	}

	void D3D12SwapChain::Present(uint32 SyncInterval)
	{
		DX_CALL(m_SwapChain->Present(SyncInterval, (SyncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0)));
		
		FRAME_INDEX = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void D3D12SwapChain::Resize(uint32 Width, uint32 Height)
	{
		ReleaseBackbuffers();

		DX_CALL(m_SwapChain->ResizeBuffers(FRAME_COUNT, Width, Height, m_SwapChainFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

		//FRAME_INDEX = 0;

		CreateBackbuffers();
	}

	void D3D12SwapChain::Initialize(D3D12Device* pDevice, D3D12Queue* pQueue, uint32 Width, uint32 Height)
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.BufferCount	= FRAME_COUNT;
		desc.Format			= m_SwapChainFormat;
		desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Width			= Width;
		desc.Height			= Height;
		desc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags			= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		desc.AlphaMode		= DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.SampleDesc		= { 1, 0 };

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{};
		fullscreenDesc.RefreshRate		= { 0, 0 };
		fullscreenDesc.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED;
		fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fullscreenDesc.Windowed			= true;

		IDXGISwapChain1* swapChain = nullptr;
		DX_CALL(pDevice->GetFactory()->CreateSwapChainForHwnd(pQueue->Get(), lde::Window::GetHWnd(), &desc, &fullscreenDesc, nullptr, &swapChain));

		DX_CALL(pDevice->GetFactory()->MakeWindowAssociation(lde::Window::GetHWnd(), DXGI_MWA_NO_ALT_ENTER));

		m_SwapChain = static_cast<IDXGISwapChain4*>(swapChain);
		SAFE_DELETE(swapChain);
		CreateBackbuffers();
		FRAME_INDEX = m_SwapChain->GetCurrentBackBufferIndex();

	}

	void D3D12SwapChain::Release()
	{
		ReleaseBackbuffers();
		SAFE_RELEASE(m_SwapChain);
	}

	void D3D12SwapChain::CreateBackbuffers()
	{
		m_DescriptorHeap->Reset();

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_DescriptorHeap->CpuStartHandle());

		for (uint32 i = 0; i < FRAME_COUNT; ++i)
		{
			DX_CALL(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_Backbuffers.at(i).GetAddressOf())));

			m_Device->GetDevice()->CreateRenderTargetView(m_Backbuffers.at(i).Get(), nullptr, rtvHandle);

			std::wstring debugName{ L"Backbuffer #" + std::to_wstring(i) };
			m_Backbuffers.at(i).Get()->SetName(debugName.c_str());

			rtvHandle.ptr += m_DescriptorHeap->GetDescriptorSize();
		}
	}

	void D3D12SwapChain::ReleaseBackbuffers()
	{
		for (uint32 i = 0; i < FRAME_COUNT; i++)
		{
			SAFE_RELEASE(m_Backbuffers.at(i));
		}
	}
}
