#pragma once

/*
	RHI/D3D12/D3D12SwapChain.hpp

*/

#include <RHI/SwapChain.hpp>
#include <AgilitySDK/d3d12.h>
#include <dxgi1_6.h>
#include "D3D12Viewport.hpp"

#include <Core/CoreMinimal.hpp>
#include <RHI/RHICommon.hpp>

#include <wrl/client.h>

namespace lde::RHI
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

		IDXGISwapChain4* Get() { return m_SwapChain.Get(); }

		void OnResize(uint32 Width, uint32 Height);

		/// @brief 
		/// @return Backbuffer for current frame.
		ID3D12Resource* GetBackbuffer()
		{
			return m_Backbuffers.at(FRAME_INDEX).Get();
		}

		D3D12DescriptorHeap* RTVHeap()
		{
			return m_Heap.get();
		}

	private:
		void Initialize(D3D12Device* pDevice, D3D12Queue* pQueue, uint32 Width, uint32 Height);
		void Release();

		void CreateBackbuffers();
		void ReleaseBackbuffers();

		ComPtr<IDXGISwapChain4> m_SwapChain;
		std::array<Ref<ID3D12Resource>, FRAME_COUNT> m_Backbuffers;
		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Output render target only
		std::unique_ptr<D3D12DescriptorHeap> m_Heap;

		D3D12Device* m_Device = nullptr; /* Parent Device */
	};
}
