#pragma once

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <RHI/RHICommon.hpp>

namespace lde::RHI
{
	class D3D12Device;
	class D3D12Queue;

	class D3D12Fence
	{
	public:
		D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
		~D3D12Fence();

		void Signal(D3D12Queue* pQueue, uint64 Value);

		void SetEvent();

		void Wait();

		bool IsValueCompleted(uint64 Value);

		/// @brief 
		/// @return Fence value for current frame.
		uint64 GetValue() const;
		/// @brief 
		/// @param AtIndex 
		/// @return Fence value at desired index.
		uint64 GetValue(uint32 AtIndex) const;

		void UpdateValue(uint64 Value);
		void UpdateValueAtIndex(usize Index);

		void OnResize();
		
	private:
		Ref<ID3D12Fence> m_Fence;
		::HANDLE m_FenceEvent = nullptr;
		std::array<uint64, FRAME_COUNT> m_FenceValues{};
	};
}
