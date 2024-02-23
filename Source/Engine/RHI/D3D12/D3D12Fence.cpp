#include <AgilitySDK/d3d12.h>
#include "D3D12Fence.hpp"
#include "D3D12Device.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	D3D12Fence::D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags)
	{
		DX_CALL(pDevice->GetDevice()->CreateFence(0, Flags, IID_PPV_ARGS(&m_Fence)));
		m_FenceValues.at(0)++;
		m_FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	D3D12Fence::~D3D12Fence()
	{
		::CloseHandle(m_FenceEvent);
		SAFE_RELEASE(m_Fence);
	}

	void D3D12Fence::Signal(D3D12Queue* pQueue, uint64 Value)
	{
		DX_CALL(pQueue->Get()->Signal(m_Fence.Get(), Value));
	}

	void D3D12Fence::SetEvent()
	{
		DX_CALL(m_Fence->SetEventOnCompletion(GetValue(), m_FenceEvent));
	}

	void D3D12Fence::Wait()
	{
		//::WaitForSingleObject(m_FenceEvent, INFINITE);
		auto value = GetValue();
		if (m_Fence->GetCompletedValue() != value)
		{
			m_Fence->SetEventOnCompletion(value, m_FenceEvent);
			::WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	bool D3D12Fence::IsValueCompleted(uint64 Value)
	{
		return m_Fence->GetCompletedValue() < Value;
	}

	uint64 D3D12Fence::GetValue() const
	{
		return m_FenceValues.at(FRAME_INDEX);
	}

	uint64 D3D12Fence::GetValue(uint32 AtIndex) const
	{
		return m_FenceValues.at(AtIndex);
	}

	void D3D12Fence::UpdateValue(uint64 Value)
	{
		m_FenceValues.at(FRAME_INDEX) = Value + 1u;
	}

	void D3D12Fence::UpdateValueAtIndex(usize Index)
	{
		m_FenceValues.at(Index)++;
	}

	void D3D12Fence::OnResize()
	{
		for (uint32 i = 0; i < FRAME_COUNT; i++)
		{
			m_FenceValues.at(i) = m_FenceValues.at(FRAME_INDEX);
		}
	}
}
