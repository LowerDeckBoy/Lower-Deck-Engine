#include "D3D12Buffer.hpp"
#include "D3D12RHI.hpp"
#include <Core/Logger.hpp>

namespace lde::RHI
{
	D3D12Descriptor D3D12Buffer::Descriptor() const
	{
		return m_Descriptor;
	}

	D3D12Buffer::D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc)
	{
		Create(pDevice, Desc);
	}

	D3D12Buffer::D3D12Buffer(D3D12Device* pDevice, D3D12UploadHeap* pUploadHeap, BufferDesc Desc)
	{
		Create(pDevice, pUploadHeap, Desc);
	}

	D3D12Buffer::~D3D12Buffer()
	{
		//Release();
	}

	void D3D12Buffer::Create(D3D12Device* pDevice, BufferDesc Desc)
	{
		if (m_Buffer.Resource.Get())
		{
			SAFE_RELEASE(m_Buffer.Resource);
			SAFE_RELEASE(m_Buffer.Allocation);
		}

		D3D12_RESOURCE_DESC desc = CreateBufferDesc(Desc.Size);

		D3D12Memory::Allocate(m_Buffer, desc, AllocType::eCopyDst);

		AllocatedResource uploadBuffer;
		D3D12Memory::Allocate(uploadBuffer, desc, AllocType::eUpload);

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = Desc.pData;
		subresource.RowPitch = static_cast<LONG_PTR>(Desc.Size);
		subresource.SlicePitch = subresource.RowPitch;

		auto* commandList = pDevice->GetGfxCommandList();

		commandList->UploadResource(uploadBuffer.Resource, m_Buffer.Resource, subresource);

		switch (Desc.eType)
		{
		case BufferUsage::eVertex:
		case BufferUsage::eConstant:
			commandList->ResourceBarrier(m_Buffer.Resource, ResourceState::eCopyDst, ResourceState::eVertexOrConstantBuffer);
			break;
		case BufferUsage::eIndex:
			commandList->ResourceBarrier(m_Buffer.Resource, ResourceState::eCopyDst, ResourceState::eIndexBuffer);
			break;
		case BufferUsage::eStructured:
			commandList->ResourceBarrier(m_Buffer.Resource, ResourceState::eCopyDst, ResourceState::eAllShaderResource);
			break;
		}

		pDevice->ExecuteCommandList(CommandType::eGraphics, true);

		SAFE_RELEASE(uploadBuffer.Allocation);
		SAFE_RELEASE(uploadBuffer.Resource);

		m_Desc = Desc;

		if (Desc.bBindless)
		{
			//pGfx->Heap->Allocate(m_Descriptor);
			pDevice->GetSRVHeap()->Allocate(m_Descriptor);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = Desc.Count;
			srvDesc.Buffer.StructureByteStride = Desc.Stride;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			pDevice->GetDevice()->CreateShaderResourceView(m_Buffer.Resource.Get(), &srvDesc, m_Descriptor.GetCpuHandle());
		}
	}

	void D3D12Buffer::Create(D3D12Device* pDevice, D3D12UploadHeap* pUploadHeap, BufferDesc Desc)
	{
		if (m_Buffer.Resource.Get())
		{
			SAFE_RELEASE(m_Buffer.Resource);
			SAFE_RELEASE(m_Buffer.Allocation);
		}

		D3D12_RESOURCE_DESC desc = CreateBufferDesc(Desc.Size);

		//D3D12Memory::Allocate(m_Buffer, desc, AllocType::eCopyDst);
		D3D12Memory::Allocate(m_Buffer, desc, AllocType::eGeneric);
		
		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = Desc.pData;
		subresource.RowPitch = static_cast<LONG_PTR>(Desc.Size);
		subresource.SlicePitch = subresource.RowPitch;

		pUploadHeap->AddBufferCopy(Desc.pData, static_cast<int32>(Desc.Size), m_Buffer.Resource.Get());

		m_Desc = Desc;

		if (Desc.bBindless)
		{
			//pGfx->Heap->Allocate(m_Descriptor);
			pDevice->GetSRVHeap()->Allocate(m_Descriptor);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = Desc.Count;
			srvDesc.Buffer.StructureByteStride = Desc.Stride;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			pDevice->GetDevice()->CreateShaderResourceView(m_Buffer.Resource.Get(), &srvDesc, m_Descriptor.GetCpuHandle());
		}
	}

	void D3D12Buffer::Release()
	{
		SAFE_RELEASE(m_Buffer.Allocation);
		SAFE_RELEASE(m_Buffer.Resource);
	}

	void* D3D12Buffer::GetCpuAddress() const
	{
		return nullptr;
	}

	uint64 D3D12Buffer::GetGpuAddress() const
	{
		return uint64();
	}

	void D3D12Buffer::Map(void* /* pMappedData */)
	{
	}

	void D3D12Buffer::Unmap()
	{
	}

	uint32 D3D12Buffer::GetSRVIndex()
	{
		return m_Descriptor.Index();
	}

	D3D12_INDEX_BUFFER_VIEW GetIndexView(D3D12Buffer* pBuffer)
	{
		if (pBuffer->GetDesc().eType != BufferUsage::eIndex)
		{
			LOG_WARN("Tried to get D3D12_INDEX_BUFFER_VIEW from invalid buffer type!");
			return D3D12_INDEX_BUFFER_VIEW();
		}

		DXGI_FORMAT format = (pBuffer->GetDesc().Stride == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		
		return D3D12_INDEX_BUFFER_VIEW(
			pBuffer->Get()->GetGPUVirtualAddress(),
			static_cast<uint32>(pBuffer->GetDesc().Size),
			format);
	}

	D3D12_VERTEX_BUFFER_VIEW GetVertexView(D3D12Buffer* pBuffer)
	{
		if (pBuffer->GetDesc().eType != BufferUsage::eVertex)
		{
			LOG_WARN("Tried to get D3D12_INDEX_BUFFER_VIEW from invalid buffer type!");
			return D3D12_VERTEX_BUFFER_VIEW();
		}
		
		return D3D12_VERTEX_BUFFER_VIEW(
			pBuffer->Get()->GetGPUVirtualAddress(),
			static_cast<uint32>(pBuffer->GetDesc().Size),
			pBuffer->GetDesc().Stride);
	}

	D3D12_RESOURCE_DESC CreateBufferDesc(usize Size, D3D12_RESOURCE_FLAGS Flag)
	{
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Format				= DXGI_FORMAT_UNKNOWN;
		desc.Width				= static_cast<uint64>(Size);
		desc.Height				= 1;
		desc.DepthOrArraySize	= 1;
		desc.MipLevels			= 1;
		desc.Alignment			= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc			= { 1, 0 };
		desc.Flags				= Flag;

		return desc;
	}

	D3D12ConstantBuffer::D3D12ConstantBuffer(void* pData, usize Size)
	{
		Create(pData, Size);
	}

	D3D12ConstantBuffer::~D3D12ConstantBuffer()
	{
		Release();
	}

	void D3D12ConstantBuffer::Create(void* pData, usize Size)
	{
		// Align data to 256 bytes
		m_Size = ALIGN(Size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);;
		
		D3D12_RESOURCE_DESC desc = CreateBufferDesc(m_Size);

		for (uint32 i = 0; i < FRAME_COUNT; i++)
		{
			m_Data.at(i) = pData;

			const auto flags = D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_COMMITTED | D3D12MA::ALLOCATION_FLAGS::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
			Ref<D3D12MA::Allocation> allocation;
			D3D12Memory::Allocate(&m_Buffers.at(i), &allocation, desc, AllocType::eDefault, flags);

			//D3D12_CONSTANT_BUFFER_VIEW_DESC bufferView{};
			//bufferView.BufferLocation = m_Buffers.at(i).Get()->GetGPUVirtualAddress();
			//bufferView.SizeInBytes = static_cast<uint32>(dataSize);

			// Persistent mapping
			const CD3DX12_RANGE readRange(0, 0);
			DX_CALL(m_Buffers.at(i)->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin.at(i))));
			std::memcpy(pDataBegin.at(i), &pData, m_Size);

			SAFE_RELEASE(allocation);

			m_Buffers.at(i)->SetName(L"Const Buffer");
		}
	}

	void D3D12ConstantBuffer::Update(void* pUpdate)
	{
		m_Data.at(FRAME_INDEX) = pUpdate;
		std::memcpy(pDataBegin.at(FRAME_INDEX), pUpdate, m_Size);
	}

	void D3D12ConstantBuffer::Release()
	{
		for (auto& buffer : m_Buffers)
			SAFE_RELEASE(buffer);
	}

}
