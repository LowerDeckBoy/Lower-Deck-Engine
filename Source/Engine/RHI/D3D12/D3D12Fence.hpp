#pragma once

#include "Core/CoreTypes.hpp"
#include "Core/RefPtr.hpp"
#include "RHI/RHICommon.hpp"
#include <vector>

namespace lde
{
	class D3D12Device;
	class D3D12Queue;

	class D3D12Fence
	{
	public:
		D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
		~D3D12Fence();

		ID3D12Fence* Get() { return m_Fence.Get(); }

		void Signal(D3D12Queue* pQueue, uint64 Value);

		void SetEventOnComplete();

		void Wait();
		void WaitForComplete();

		bool IsValueCompleted(uint64 Value);

		void UpdateValue(uint64 Value);

		void OnResize();

		uint64 GetCurrentValue()
		{
			return Values.at(FRAME_INDEX);
		}

		std::vector<uint64> Values;
		std::vector<uint64> SignaledValues;

		::HANDLE m_FenceEvent = nullptr;

	private:
		Ref<ID3D12Fence> m_Fence;

	};

	//class D3D12Fence
	//{
	//public:
	//	D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAG_NONE);
	//	~D3D12Fence();
	//
	//	ID3D12Fence* Get() { return m_Fence.Get(); }
	//
	//	void Signal(D3D12Queue* pQueue, uint64 Value);
	//
	//	void SetEvent();
	//
	//	//void SetEvent(uint64 Value);
	//
	//	void Wait();
	//
	//	//void Wait(uint64 Value);
	//
	//	bool IsValueCompleted(uint64 Value);
	//
	//	/**
	//	 * @brief 
	//	 * @return Fence value for current frame.
	//	 */
	//	uint64 GetValue() const;
	//	/**
	//	 * @brief 
	//	 * @param AtIndex 
	//	 * @return Fence value at desired index.
	//	 */
	//	uint64 GetValue(uint32 AtIndex) const;
	//
	//	void UpdateValue(uint64 Value);
	//	void UpdateValueAtIndex(usize Index);
	//
	//	void OnResize();
	//	//void OnResize(uint64 Values[FRAME_COUNT]);
	//	
	//private:
	//	Ref<ID3D12Fence> m_Fence;
	//	::HANDLE m_FenceEvent = nullptr;
	//	std::array<uint64, FRAME_COUNT> m_FenceValues{};
	//};
	
} // namespace lde
