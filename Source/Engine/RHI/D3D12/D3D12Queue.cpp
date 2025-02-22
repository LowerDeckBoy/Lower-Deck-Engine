#include "D3D12Device.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Utility.hpp"
#include "RHI/Types.hpp"

namespace lde
{
	D3D12Queue::D3D12Queue(D3D12Device* pDevice, CommandType eType, std::string_view DebugName)
		: m_Fence(pDevice)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask = pDevice->NodeMask;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

		switch (eType)
		{
		case CommandType::eGraphics:
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case CommandType::eUpload:
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case CommandType::eCompute:
			desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		case CommandType::eBundle:
			desc.Type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
			break;
		}

		Type = eType;

		DX_CALL(pDevice->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_Queue)));
		SET_D3D12_NAME(m_Queue, DebugName);

	}

	D3D12Queue::~D3D12Queue()
	{
		SAFE_RELEASE(m_Queue);
	}

	void D3D12Queue::Execute(D3D12CommandList* pCommandList, bool bReset)
	{
		pCommandList->Close();

		ID3D12CommandList* commandLists[1] = { pCommandList->Get() };

		m_Queue->ExecuteCommandLists(1, &commandLists[1]);

		if (bReset)
		{
			pCommandList->ResetList();
		}

		SignalAndWait();
	}

	void D3D12Queue::SignalFence()
	{
		m_Fence.Get()->Signal(m_Fence.GetCurrentValue());
	}

	void D3D12Queue::SignalFence(uint64 Value)
	{
		DX_CALL(m_Fence.Get()->Signal(Value));
	}

	void D3D12Queue::WaitForComplete()
	{
		m_Fence.WaitForComplete();
	}

	void D3D12Queue::Wait()
	{
		
		m_Fence.SetEventOnComplete();
		::WaitForSingleObject(m_Fence.m_FenceEvent, INFINITE);

	}

	void D3D12Queue::SignalAndWait()
	{
		//if (m_Fence.CurrentValue == 0)
		//{
		//	++Fence.LastSignaledValue;
		//}

		m_Queue->Signal(m_Fence.Get(), m_Fence.GetCurrentValue());

		if (m_Fence.SignaledValues.at(FRAME_INDEX) <= m_Fence.GetCurrentValue())
		{
			m_Fence.Wait();

			m_Fence.Values.at(FRAME_INDEX)++;
			m_Fence.SignaledValues.at(FRAME_INDEX) = m_Fence.Values.at(FRAME_INDEX);
		}
	}

	/*
	D3D12Queue::D3D12Queue(D3D12Device* pDevice, CommandType eType, std::string_view DebugName)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask = pDevice->NodeMask;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

		switch (eType)
		{
		case CommandType::eGraphics:
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case CommandType::eUpload:
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case CommandType::eCompute:
			desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		case CommandType::eBundle:
			desc.Type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
			break;
		}

		Type = eType;

		DX_CALL(pDevice->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_Queue)));
		SET_D3D12_NAME(m_Queue, DebugName);
		
	}

	D3D12Queue::~D3D12Queue()
	{
		SAFE_RELEASE(m_Queue);
	}

	void D3D12Queue::Signal(D3D12Fence* pFence, uint64 Value)
	{
		//DX_CALL(m_Queue->Signal(pFence->Get(), Value));
	}

	void D3D12Queue::Wait(D3D12Fence* pFence, uint64 Value)
	{
		//pFence->Wait(Value);
		//pFence->Wait();
	}

	void D3D12Queue::SignalAndWait(D3D12Fence* pFence, uint64 Value)
	{
		Signal(pFence, Value);
		Wait(pFence, Value);
	}
	*/
}
