#include "D3D12Queue.hpp"
#include "D3D12Device.hpp"
#include "RHI/Types.hpp"
#include "D3D12Utility.hpp"

namespace mf::RHI
{
	D3D12Queue::D3D12Queue(D3D12Device* pDevice, CommandType eType, LPCWSTR DebugName)
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.NodeMask = 0;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		//desc.Priority = pDevice->CommandQueuePriority;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

		switch (eType)
		{
		case CommandType::eGraphics:
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
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

		if (DebugName)
			m_Queue->SetName(DebugName);

	}

	D3D12Queue::~D3D12Queue()
	{
		SAFE_RELEASE(m_Queue);
	}

}
