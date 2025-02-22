#include <AgilitySDK/d3d12.h>
#include "D3D12Fence.hpp"
#include "D3D12Device.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Utility.hpp"

namespace lde
{

	D3D12Fence::D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags)
	{
		DX_CALL(pDevice->GetDevice()->CreateFence(0, Flags, IID_PPV_ARGS(&m_Fence)));
		m_FenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

		Values.resize(FRAME_COUNT);
		SignaledValues.resize(FRAME_COUNT);

		++Values.at(0);

	}

	D3D12Fence::~D3D12Fence()
	{
	}

	void D3D12Fence::SetEventOnComplete()
	{
		m_Fence->SetEventOnCompletion(Values.at(FRAME_INDEX), m_FenceEvent);
	}
	
	void D3D12Fence::Wait()
	{
		SetEventOnComplete();
		::WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	void D3D12Fence::WaitForComplete()
	{
		if (SignaledValues.at(FRAME_INDEX) < Values.at(FRAME_INDEX))
		{
			SetEventOnComplete();
			::WaitForSingleObject(m_FenceEvent, INFINITE);

			++Values.at(FRAME_INDEX);
			SignaledValues.at(FRAME_INDEX) = Values.at(FRAME_INDEX);
		}
	}
	{
	}

	void D3D12Fence::OnResize()
	{
	}




	/*
	D3D12Fence::D3D12Fence(D3D12Device* pDevice, D3D12_FENCE_FLAGS Flags)
	{
		DX_CALL(pDevice->GetDevice()->CreateFence(0, Flags, IID_PPV_ARGS(&m_Fence)));
		++m_FenceValues.at(0);
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
	*/

	/*
	void D3D12Fence::SetEvent(uint64 Value)
	{
		DX_CALL(m_Fence->SetEventOnCompletion(Value, m_FenceEvent));
	}

	void D3D12Fence::Wait(uint64 Value)
	{
		if (IsValueCompleted(Value))
		{
			m_Fence->SetEventOnCompletion(Value, m_FenceEvent);
			::WaitForSingleObject(m_FenceEvent, INFINITE);
		}
	}

	bool D3D12Fence::IsValueCompleted(uint64 Value)
	{
		return m_Fence->GetCompletedValue() < Value;
	}

	void D3D12Fence::UpdateValue(uint64 Value)
	{
		Values.at(FRAME_INDEX) = Value;
	}

	void D3D12Fence::OnResize()
	{
	}
}
