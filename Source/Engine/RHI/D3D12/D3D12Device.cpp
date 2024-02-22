#include "D3D12Device.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12Texture.hpp"

namespace lde::RHI
{
	void D3D12Device::WaitForGPU()
	{
		m_Fence->Signal(m_GfxQueue.get(), m_Fence->GetValue());

		//m_Fence->SetEvent();
		m_Fence->Wait();

		m_Fence->UpdateValue(m_Fence->GetValue());
	}

	void D3D12Device::FlushGPU()
	{
		ID3D12CommandQueue* queue = m_GfxQueue->Get();
		ID3D12Fence* pFence;
		DX_CALL(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));

		DX_CALL(queue->Signal(pFence, 1));

		HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		pFence->SetEventOnCompletion(1, fenceEvent);
		if (fenceEvent)
		{
			::WaitForSingleObject(fenceEvent, INFINITE);
			::CloseHandle(fenceEvent);
		}

		FRAME_INDEX = 0;
		SAFE_DELETE(pFence);
	}

	void D3D12Device::IdleGPU()
	{
		WaitForGPU();
		FlushGPU();
	}

	void D3D12Device::ExecuteCommandList(CommandType eType, bool bResetAllocator)
	{
		D3D12CommandList* commandList = nullptr;
		ID3D12CommandQueue* commandQueue = nullptr;

		switch (eType)
		{
		case lde::RHI::CommandType::eGraphics:
			commandList = m_GfxCommandList.get();
			commandQueue = m_GfxQueue->Get();
			break;
		case lde::RHI::CommandType::eCompute:
			commandList = m_ComputeCommandList.get();
			commandQueue = m_ComputeQueue->Get();
			break;
		case lde::RHI::CommandType::eUpload:
			commandList = m_UploadCommandList.get();
			commandQueue = m_UploadQueue->Get();
			break;
		case lde::RHI::CommandType::eBundle:
			break;
		}

		DX_CALL(commandList->Get()->Close());
		
		std::array<ID3D12CommandList*, 1> commandLists{ commandList->Get() };
		
		commandQueue->ExecuteCommandLists(static_cast<uint32>(commandLists.size()), commandLists.data());
		
		if (bResetAllocator)
		{
			commandList->ResetList();
		}
		
		WaitForGPU();
	}


    Buffer* D3D12Device::CreateBuffer(BufferDesc Desc)
    {
        return nullptr;
    }

    ConstantBuffer* D3D12Device::CreateConstantBuffer(void* pData, usize Size)
    {
        return nullptr;
    }

    Texture* D3D12Device::CreateTexture(TextureDesc Desc)
    {
        return nullptr;
    }
} // namespace lde::RHI
