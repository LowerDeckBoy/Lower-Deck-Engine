#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	D3D12Device::D3D12Device()
	{
		Create();
	}

	D3D12Device::~D3D12Device()
	{
		Release();
	}

	//void D3D12Device::WaitForGPU()
	//{
	//	m_Fence->Signal(GetFrameResources().GraphicsQueue, m_Fence->GetValue());
	//
	//	m_Fence->Wait();
	//
	//	//m_Fence->UpdateValue(GetFrameResources().FrameFenceValue);
	//	m_Fence->UpdateValue(m_Fence->GetValue());
	//}

	void D3D12Device::WaitForGPU(CommandType eType)
	{
		switch (eType)
		{
		case lde::RHI::CommandType::eGraphics:
			m_Fence->Signal(GetGfxQueue(), m_Fence->GetValue());
			//m_Fence->Signal(GetFrameResources().GraphicsQueue, m_Fence->GetValue());
			//m_FrameResources.GraphicsQueue->Signal(m_Fence.get(), m_FrameResources.RenderFenceValues[FRAME_INDEX]);
			break;
		}

		m_Fence->Wait();
		//m_Fence->Wait(m_FrameResources.RenderFenceValues[FRAME_INDEX]);

		m_Fence->UpdateValue(m_Fence->GetValue());
		//m_FrameResources.GraphicsQueue->Wait(m_Fence.get(), m_FrameResources.RenderFenceValues[FRAME_INDEX]);
		//m_FrameResources.RenderFenceValues[FRAME_INDEX]++;
	}

	void D3D12Device::FlushGPU()
	{
		//ID3D12CommandQueue* queue = m_GfxQueue->Get();
		//ID3D12CommandQueue* queue = GetFrameResources().GraphicsQueue->Get();
		ID3D12CommandQueue* queue = GetGfxQueue()->Get();
		ID3D12Fence* pFence = nullptr;
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
		WaitForGPU(CommandType::eGraphics);
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
			commandList  = GetGfxCommandList();
			commandQueue = GetGfxQueue()->Get();
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
		
		//WaitForGPU();
		WaitForGPU(eType);
	}

	void D3D12Device::CreateFrameResources()
	{
		m_FrameResources.GraphicsCommandList = new D3D12CommandList(this, CommandType::eGraphics);
		m_FrameResources.GraphicsQueue = new D3D12Queue(this, CommandType::eGraphics);
		////m_FrameResources.RenderFenceValue = 1;
		m_FrameResources.RenderFenceValues[0] = 1;
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

	BufferHandle D3D12Device::CreateBuffer(BufferDesc Desc)
	{
		BufferHandle handle = static_cast<BufferHandle>(Buffers.size());

		Buffers.push_back(new D3D12Buffer(this, Desc));
		
		return handle;
	}
	
	BufferHandle D3D12Device::CreateConstantBuffer(void* pData, usize Size)
	{
		BufferHandle handle = static_cast<BufferHandle>(ConstantBuffers.size());

		ConstantBuffers.push_back(new D3D12ConstantBuffer(pData, Size));

		return handle;
	}

	TextureHandle D3D12Device::CreateTexture(D3D12Texture* pTexture)
	{
		TextureHandle handle = static_cast<TextureHandle>(Textures.size());

		Textures.push_back(pTexture);

		return handle;
	}

	void D3D12Device::DestroyBuffer(BufferHandle Handle)
	{
		Buffers.at(Handle)->Release();
		delete Buffers.at(Handle);
	}

	void D3D12Device::DestroyConstantBuffer(BufferHandle Handle)
	{
		ConstantBuffers.at(Handle)->Release();
		delete ConstantBuffers.at(Handle);
	}

	void D3D12Device::DestroyTexture(TextureHandle Handle)
	{
		Textures.at(Handle)->Release();
		delete Textures.at(Handle);
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
