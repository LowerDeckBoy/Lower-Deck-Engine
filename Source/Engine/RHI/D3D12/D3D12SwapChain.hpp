#pragma once


#include <AgilitySDK/d3d12.h>
#include "RHI/SwapChain.hpp"
#include <dxgi1_6.h>
#include "Core/CoreMinimal.hpp"
#include "RHI/RHICommon.hpp"
#include <wrl/client.h>
#include "Config.hpp"

namespace lde
{
	class D3D12Device;
	class D3D12Queue;
	class D3D12DescriptorHeap;

	using Microsoft::WRL::ComPtr;

	class D3D12SwapChain : public SwapChain
	{
	public:
		D3D12SwapChain(D3D12Device* pDevice, D3D12Queue* pQueue, uint32 Width, uint32 Height);
		~D3D12SwapChain();

		IDXGISwapChain4* Get()
		{
			return m_SwapChain.Get();
		}
		
		void Present(uint32 SyncInterval);

		void Resize(uint32 Width, uint32 Height);

		ID3D12Resource* GetBackbuffer()
		{
			return m_Backbuffers.at(FRAME_INDEX).Get();
		}

		D3D12DescriptorHeap* RTVHeap()
		{
			return m_DescriptorHeap.get();
		}

	private:
		void Initialize(D3D12Device* pDevice, D3D12Queue* pQueue, uint32 Width, uint32 Height);
		void Release();

		void CreateBackbuffers();
		void ReleaseBackbuffers();

		ComPtr<IDXGISwapChain4> m_SwapChain;

		std::array<Ref<ID3D12Resource>, FRAME_COUNT> m_Backbuffers;

		DXGI_FORMAT m_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Output render target only.
		std::unique_ptr<D3D12DescriptorHeap> m_DescriptorHeap;

		D3D12Device* m_Device = nullptr;

	};
} // namespace lde
