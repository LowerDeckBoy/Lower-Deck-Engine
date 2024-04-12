#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	void D3D12Device::WaitForGPU()
	{
		m_Fence->Signal(GetFrameResources().GraphicsQueue, m_Fence->GetValue());
		//m_Fence->Signal(GetFrameResources().GraphicsQueue, GetFrameResources().FrameFenceValue);
		//m_Fence->Signal(m_GfxQueue.get(), m_Fence->GetValue());

		//m_Fence->SetEvent();
		m_Fence->Wait();

		//m_Fence->UpdateValue(GetFrameResources().FrameFenceValue);
		m_Fence->UpdateValue(m_Fence->GetValue());
	}

	void D3D12Device::WaitForGPU(CommandType eType)
	{
		switch (eType)
		{
		case lde::RHI::CommandType::eGraphics:
			m_Fence->Signal(GetFrameResources().GraphicsQueue, m_Fence->GetValue());
			break;
		//case lde::RHI::CommandType::eCompute:
		//	break;
		//case lde::RHI::CommandType::eUpload:
		//	break;
		//case lde::RHI::CommandType::eBundle:
		//	break;
		}

		m_Fence->Wait();

		m_Fence->UpdateValue(m_Fence->GetValue());
	}

	void D3D12Device::FlushGPU()
	{
		//ID3D12CommandQueue* queue = m_GfxQueue->Get();
		ID3D12CommandQueue* queue = GetFrameResources().GraphicsQueue->Get();
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
		{
			commandList = GetFrameResources().GraphicsCommandList;
			commandQueue = GetFrameResources().GraphicsQueue->Get();
			break;
		}
		default:
			throw std::runtime_error("");
		}
	
		DX_CALL(commandList->Close());
		
		ID3D12CommandList* commandLists[] = { commandList->Get() };
		
		commandQueue->ExecuteCommandLists(1, commandLists);
		
		if (bResetAllocator)
		{
			commandList->ResetList();
		}
		
		WaitForGPU();
	}

	void D3D12Device::CreateFrameResources()
	{
		m_FrameResources.GraphicsCommandList = new D3D12CommandList(this, CommandType::eGraphics);
		m_FrameResources.GraphicsQueue = new D3D12Queue(this, CommandType::eGraphics);
		m_FrameResources.FrameFenceValue = 0;

		m_FrameResources.ComputeCommandList = new D3D12CommandList(this, CommandType::eCompute);
		m_FrameResources.ComputeQueue = new D3D12Queue(this, CommandType::eCompute);
		m_FrameResources.ComputeFenceValue = 0;

		m_FrameResources.UploadCommandList = new D3D12CommandList(this, CommandType::eUpload);
		m_FrameResources.UploadQueue = new D3D12Queue(this, CommandType::eUpload);
		m_FrameResources.UploadFenceValue = 0;

	}

	void D3D12Device::Allocate(HeapType eType, D3D12Descriptor& Descriptor, uint32 Count)
	{
		switch (eType)
		{
		case lde::RHI::HeapType::eSRV: 
			m_SRVHeap->Allocate(Descriptor, Count);
			break;
		case lde::RHI::HeapType::eRTV:
			m_RTVHeap->Allocate(Descriptor, Count);
			break;
		case lde::RHI::HeapType::eDSV:
			m_DSVHeap->Allocate(Descriptor, Count);
			break;
		}
	}

    Buffer* D3D12Device::CreateBuffer(BufferDesc Desc)
    {
        return new D3D12Buffer(this, Desc);
    }

    ConstantBuffer* D3D12Device::CreateConstantBuffer(void* pData, usize Size)
    {
        return new D3D12ConstantBuffer(pData, Size);
    }

    Texture* D3D12Device::CreateTexture(TextureDesc /* Desc */)
    {
        return nullptr;
    }

	void D3D12Device::CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count)
	{
		const auto desc = pResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = desc.Format;
		
		if (desc.DepthOrArraySize == 6)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = desc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = desc.MipLevels;
			srvDesc.Texture3D.MostDetailedMip = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			// TODO:
		}
		
		Descriptor = m_SRVHeap->Allocate(Count);
		m_Device->CreateShaderResourceView(pResource, &srvDesc, Descriptor.GetCpuHandle());
	}

	void D3D12Device::CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Mips, uint32 Count)
	{
		const auto desc = pResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = desc.Format;

		if (desc.DepthOrArraySize == 6)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = Mips;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = Mips;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.PlaneSlice = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = Mips;
			srvDesc.Texture3D.MostDetailedMip = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			// TODO:
		}

		Descriptor = m_SRVHeap->Allocate(Count);
		m_Device->CreateShaderResourceView(pResource, &srvDesc, Descriptor.GetCpuHandle());
	}

	void D3D12Device::CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count)
	{
		const auto desc = pResource->GetDesc();
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = desc.Format;

		if (desc.DepthOrArraySize > 1)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = 0;
			uavDesc.Texture2DArray.PlaneSlice = 0;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = 0;
			uavDesc.Texture2D.PlaneSlice = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.MipSlice = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			// TODO:
		}

		Descriptor = m_SRVHeap->Allocate(Count);
		m_Device->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, Descriptor.GetCpuHandle());

	}

	void D3D12Device::CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 MipSlice, uint32 Count)
	{
		const auto desc = pResource->GetDesc();
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = desc.Format;

		if (desc.DepthOrArraySize > 1)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.MipSlice = MipSlice;
			uavDesc.Texture2DArray.PlaneSlice = 0;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.MipSlice = MipSlice;
			uavDesc.Texture2D.PlaneSlice = 0;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture3D.MipSlice = MipSlice;
		}
		else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			// TODO:
		}

		Descriptor = m_SRVHeap->Allocate(Count);
		m_Device->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, Descriptor.GetCpuHandle());

	}
} // namespace lde::RHI
