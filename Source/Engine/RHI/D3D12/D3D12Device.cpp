#include "D3D12Buffer.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"

namespace lde
{
	D3D12Device::D3D12Device()
	{
		Create();
	}

	D3D12Device::~D3D12Device()
	{
		Release();
	}

	void D3D12Device::WaitForGPU(CommandType eType)
	{
		//GraphicsQueue->SignalAndWait();
		
		switch (eType)
		{
		case CommandType::eGraphics:
			GraphicsQueue->SignalAndWait();
			break;
		case CommandType::eCompute:
			break;
		}
	}

	void D3D12Device::FlushGPU()
	{
		ID3D12Fence* pFence = nullptr;
		DX_CALL(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));

		DX_CALL(GetGfxQueue()->Get()->Signal(pFence, 1));

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
		case CommandType::eGraphics:
		{
			commandList  = GetGfxCommandList();
			commandQueue = GetGfxQueue()->Get();
			break;
		}
		//case CommandType::eCompute:
		//{
		//	commandList  = GetComputeCommandList();
		//	commandQueue = GetComputeQueue()->Get();
		//	break;
		//}
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
		
		WaitForGPU(eType);
	}

	void D3D12Device::ExecuteAllCommandLists(bool bResetAllocators)
	{
		ID3D12CommandList* commandLists[FRAME_COUNT]{};

		for (usize frame = 0; frame < FRAME_COUNT; ++frame)
		{
			auto commandList = m_FrameResources[frame].GraphicsCommandList;

			commandLists[frame] = commandList->Get();

			DX_CALL(commandList->Close());

			if (bResetAllocators)
			{
				commandList->ResetList();
			}
		}

		GetGfxQueue()->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForGPU(CommandType::eGraphics);
	}

	void D3D12Device::CreateFrameResources()
	{
		for (usize frame = 0; frame < FRAME_COUNT; frame++)
		{
			m_FrameResources[frame].GraphicsCommandList = new D3D12CommandList(this, CommandType::eGraphics, std::format("D3D12 Graphics Command List #{}", frame).c_str());

			//m_FrameResources[frame].ComputeCommandList = new D3D12CommandList(this, CommandType::eCompute, std::format("D3D12 Compute Command List #{}", frame).c_str());
		}

		GraphicsQueue = new D3D12Queue(this, CommandType::eGraphics);
		//ComputeQueue = new D3D12Queue(this, CommandType::eCompute);

		// Open first command list to allow pre-loading of assets - models and skybox + ibl.
		m_FrameResources[0].GraphicsCommandList->Open();
		//m_FrameResources[0].ComputeCommandList->Reset();
	}

	void D3D12Device::Allocate(HeapType eType, D3D12Descriptor& Descriptor, uint32 Count)
	{
		switch (eType)
		{
		case lde::HeapType::eSRV: 
			m_ShaderResourceHeap->Allocate(Descriptor, Count);
			break;
		case lde::HeapType::eRTV:
			m_RenderTargetHeap->Allocate(Descriptor, Count);
			break;
		case lde::HeapType::eDSV:
			m_DepthStencilHeap->Allocate(Descriptor, Count);
			break;
		}
	}

	D3D12Buffer* D3D12Device::GetBuffer(uint32 Index)
	{
		return Buffers.at(Index);
	}

	D3D12ConstantBuffer* D3D12Device::GetConstantBuffer(uint32 Index)
	{
		return ConstantBuffers.at(Index);
	}

	D3D12Texture* D3D12Device::GetTexture(uint32 Index)
	{
		return Textures.at(Index);
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

	TextureHandle D3D12Device::CreateTexture(TextureDesc Desc)
	{
		TextureHandle handle = static_cast<TextureHandle>(Textures.size());

		D3D12Texture* texture;

		Textures.push_back(texture);

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

		m_ShaderResourceHeap->Allocate(Descriptor, Count);
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

		//m_ShaderResourceHeap->Allocate(Descriptor, Count);
		Descriptor = m_ShaderResourceHeap->Allocate(Count);
		m_Device->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, Descriptor.GetCpuHandle());
	}

	void D3D12Device::CreateRTV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format					= Format;
		rtvDesc.ViewDimension			= D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice		= 0;
		rtvDesc.Texture2D.PlaneSlice	= 0;

		m_RenderTargetHeap->Allocate(Descriptor);
		m_Device->CreateRenderTargetView(pResource, &rtvDesc, Descriptor.GetCpuHandle());
	}

	void D3D12Device::CreateDSV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format				= Format;
		dsvDesc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice	= 0;
		dsvDesc.Flags				= D3D12_DSV_FLAG_NONE;

		m_DepthStencilHeap->Allocate(Descriptor);
		m_Device->CreateDepthStencilView(pResource, &dsvDesc, Descriptor.GetCpuHandle());
	}

} // namespace lde
