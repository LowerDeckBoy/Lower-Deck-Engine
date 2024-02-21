#include "D3D12Device.hpp"
#include "D3D12Buffer.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Utility.hpp"
#include "RHI/Types.hpp"

namespace mf::RHI
{
	D3D12CommandList::D3D12CommandList(D3D12Device* pDevice, CommandType eType, std::string DebugName)
	{
		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_NONE;
		switch (eType)
		{
		case CommandType::eGraphics:	type = D3D12_COMMAND_LIST_TYPE_DIRECT;	break;
		case CommandType::eCompute:		type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
		case CommandType::eUpload:		type = D3D12_COMMAND_LIST_TYPE_COPY;	break;
		case CommandType::eBundle:		type = D3D12_COMMAND_LIST_TYPE_BUNDLE;	break;
		}

		for (auto& allocator : m_Allocators)
		{
			DX_CALL(pDevice->GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));
		}
		DX_CALL(pDevice->GetDevice()->CreateCommandList(0, type, GetAllocator(), nullptr, IID_PPV_ARGS(&m_CommandList)));
		SET_NAME(m_CommandList, DebugName);

		m_Type = eType;
	}

	D3D12CommandList::~D3D12CommandList()
	{
		SAFE_RELEASE(m_CommandList);
		for (auto& allocator : m_Allocators)
		{
			SAFE_RELEASE(allocator);
		}
	}

	void D3D12CommandList::ResetAllocator()
	{
		DX_CALL(GetAllocator()->Reset());
	}

	void D3D12CommandList::ResetList()
	{
		DX_CALL(m_CommandList->Reset(GetAllocator(), nullptr));
	}

	void D3D12CommandList::Reset()
	{
		DX_CALL(GetAllocator()->Reset());
		DX_CALL(m_CommandList->Reset(GetAllocator(), nullptr));
	}
	 
	void D3D12CommandList::DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex)
	{
		m_CommandList->DrawIndexedInstanced(1, IndexCount, BaseIndex, BaseVertex, 0);
	}

	void D3D12CommandList::DrawIndexedInstanced(uint32 Instances, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex)
	{
		m_CommandList->DrawIndexedInstanced(Instances, IndexCount, BaseIndex, BaseVertex, 0);
	}

	void D3D12CommandList::Draw(uint32 VertexCount)
	{
		m_CommandList->DrawInstanced(1, VertexCount, 0, 0);
	}

	void D3D12CommandList::BindVertexBuffer(Buffer* pBuffer)
	{
		const auto view = GetVertexView((D3D12Buffer*)pBuffer);
		m_CommandList->IASetVertexBuffers(0, 1, &view);
	}

	void D3D12CommandList::BindIndexBuffer(Buffer* pBuffer)
	{
		const auto view = GetIndexView((D3D12Buffer*)pBuffer);
		m_CommandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandList::BindConstantBuffer(uint32 Slot, ConstantBuffer* pBuffer)
	{
		const auto address = ((D3D12ConstantBuffer*)pBuffer)->GetBuffer()->GetGPUVirtualAddress();
		if (m_Type == CommandType::eGraphics)
		{
			m_CommandList->SetGraphicsRootConstantBufferView(Slot, address);
		}
		else if (m_Type == CommandType::eCompute)
		{
			m_CommandList->SetGraphicsRootConstantBufferView(Slot, address);
		}
	}

	void D3D12CommandList::PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset)
	{
		if (m_Type == CommandType::eGraphics)
		{
			m_CommandList->SetGraphicsRoot32BitConstants(Slot, Count, &pData, Offset);
		}
		else if (m_Type == CommandType::eCompute)
		{
			m_CommandList->SetComputeRoot32BitConstants(Slot, Count, &pData, Offset);
		}
	}

	

} // namespace mf::RHI
