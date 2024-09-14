#include "D3D12Device.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Utility.hpp"
#include "RHI/Types.hpp"

namespace lde
{
	D3D12Queue::D3D12Queue(D3D12Device* pDevice, CommandType eType, std::string_view DebugName)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask = DEVICE_NODE;
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
	
}
