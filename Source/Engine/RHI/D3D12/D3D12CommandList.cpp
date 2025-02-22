#include "D3D12Buffer.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Device.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Utility.hpp"
#include "RHI/Types.hpp"
#include <AgilitySDK/d3dx12/d3dx12_resource_helpers.h>

namespace lde
{
	static constexpr D3D12_COMMAND_LIST_TYPE GetCommandType(CommandType eType)
	{
		switch (eType)
		{
		case lde::CommandType::eGraphics:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case lde::CommandType::eCompute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case lde::CommandType::eUpload:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		case lde::CommandType::eBundle:
			return D3D12_COMMAND_LIST_TYPE_BUNDLE;
		default:
			return D3D12_COMMAND_LIST_TYPE_NONE;
		}
	}

	/* =============================== Command Allocator =============================== */

	D3D12CommandAllocator::D3D12CommandAllocator(D3D12Device* pDevice, CommandType eType)
	{
		Type = eType;
		DX_CALL(pDevice->GetDevice()->CreateCommandAllocator(GetCommandType(eType), IID_PPV_ARGS(&m_Allocator)));
	}

	D3D12CommandAllocator::~D3D12CommandAllocator()
	{
		SAFE_RELEASE(m_Allocator);
	}

	void D3D12CommandAllocator::Reset()
	{
		DX_CALL(m_Allocator->Reset());
	}
	
	/* =============================== Command List =============================== */

	D3D12CommandList::D3D12CommandList(D3D12Device* pDevice, CommandType eType, std::string DebugName)
	{
		D3D12_COMMAND_LIST_TYPE type = GetCommandType(eType);

		m_Allocator = new D3D12CommandAllocator(pDevice, eType);
		DX_CALL(pDevice->GetDevice()->CreateCommandList1(pDevice->NodeMask, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_GraphicsCommandList)));

		SET_D3D12_NAME(m_GraphicsCommandList, DebugName);
		//SET_D3D12_NAME(*m_Allocator, (DebugName + ": Allocator"));

		m_Type = eType;
	}

	D3D12CommandList::~D3D12CommandList()
	{
		SAFE_RELEASE(m_GraphicsCommandList);
		delete m_Allocator;
	}

	//void D3D12CommandList::ResetAllocator()
	//{
	//	m_Allocator->Reset();
	//}

	void D3D12CommandList::ResetList()
	{
		DX_CALL(m_GraphicsCommandList->Reset(m_Allocator->Get(), nullptr));
	}

	HRESULT D3D12CommandList::Open()
	{
		HRESULT hResult = m_Allocator->Get()->Reset();

		if (FAILED(hResult))
		{
			// log
		}

		hResult = m_GraphicsCommandList->Reset(m_Allocator->Get(), nullptr);

		return hResult;
	}

	HRESULT D3D12CommandList::Close()
	{
		return m_GraphicsCommandList->Close();
	}

	void D3D12CommandList::DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex)
	{
		m_GraphicsCommandList->DrawIndexedInstanced(IndexCount, 1, BaseIndex, BaseVertex, 0);
	}

	void D3D12CommandList::DrawIndexedInstanced(uint32 Instances, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex)
	{
		m_GraphicsCommandList->DrawIndexedInstanced(IndexCount, Instances, BaseIndex, BaseVertex, 0);
	}

	void D3D12CommandList::Draw(uint32 VertexCount)
	{
		m_GraphicsCommandList->DrawInstanced(VertexCount, 1, 0, 0);
	}

	void D3D12CommandList::DrawIndirect()
	{
		//m_GraphicsCommandList->ExecuteIndirect()
	}

	void D3D12CommandList::DispatchRays(const D3D12_DISPATCH_RAYS_DESC& Desc)
	{
		m_GraphicsCommandList->DispatchRays(&Desc);
	}

	void D3D12CommandList::DispatchMesh(uint32 DispatchX, uint32 DispatchY, uint32 DispatchZ)
	{
		m_GraphicsCommandList->DispatchMesh(DispatchX, DispatchY, DispatchZ);
	}

	void D3D12CommandList::BindVertexBuffer(Buffer* pBuffer)
	{
		const auto& view = GetVertexView((D3D12Buffer*)pBuffer);
		m_GraphicsCommandList->IASetVertexBuffers(0, 1, &view);
	}

	void D3D12CommandList::BindIndexBuffer(Buffer* pBuffer)
	{
		const auto& view = GetIndexView((D3D12Buffer*)pBuffer);
		m_GraphicsCommandList->IASetIndexBuffer(&view);
	}

	void D3D12CommandList::BindConstantBuffer(uint32 Slot, ConstantBuffer* pBuffer)
	{
		const auto address = ((D3D12ConstantBuffer*)pBuffer)->GetBuffer()->GetGPUVirtualAddress();
		if (m_Type == CommandType::eGraphics)
		{
			m_GraphicsCommandList->SetGraphicsRootConstantBufferView(Slot, address);
		}
		else if (m_Type == CommandType::eCompute)
		{
			m_GraphicsCommandList->SetGraphicsRootConstantBufferView(Slot, address);
		}
	}

	void D3D12CommandList::PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset)
	{
		if (m_Type == CommandType::eGraphics)
		{
			m_GraphicsCommandList->SetGraphicsRoot32BitConstants(Slot, Count, pData, Offset);
		}
		else if (m_Type == CommandType::eCompute)
		{
			m_GraphicsCommandList->SetComputeRoot32BitConstants(Slot, Count, pData, Offset);
		}
	}

	void D3D12CommandList::ResourceBarrier(Ref<ID3D12Resource> ppResource, ResourceState Before, ResourceState After)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource	= ppResource.Get();
		barrier.Transition.StateBefore	= StateEnumToType(Before);
		barrier.Transition.StateAfter	= StateEnumToType(After);

		m_GraphicsCommandList->ResourceBarrier(1, &barrier);
	}

	void D3D12CommandList::ResourceBarriers(std::span<D3D12_RESOURCE_BARRIER> Barriers)
	{
		m_GraphicsCommandList->ResourceBarrier(static_cast<uint32>(Barriers.size()), Barriers.data());
	}

	void D3D12CommandList::UploadResource(Ref<ID3D12Resource> ppSrc, Ref<ID3D12Resource> ppDst, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(m_GraphicsCommandList.Get(), ppDst.Get(), ppSrc.Get(), 0, 0, 1, &Subresource);
	}

	D3D12_RESOURCE_STATES StateEnumToType(ResourceState eState)
	{
		switch (eState)
		{
		case lde::ResourceState::eGeneralUsage:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		case lde::ResourceState::eRenderTarget:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case lde::ResourceState::ePresent:
			return D3D12_RESOURCE_STATE_PRESENT;
		case lde::ResourceState::eCopySrc:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case lde::ResourceState::eCopyDst:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case lde::ResourceState::eVertexOrConstantBuffer:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case lde::ResourceState::eIndexBuffer:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case lde::ResourceState::eAllShaderResource:
			return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		case lde::ResourceState::ePixelShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		default:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		}
	}
	
	/* =============================== Command Signature =============================== */

	D3D12CommandSignature::D3D12CommandSignature()
	{
		
	}

	HRESULT D3D12CommandSignature::Create(D3D12Device* pDevice, D3D12RootSignature* pRootSignature)
	{
		D3D12_COMMAND_SIGNATURE_DESC desc{};
		desc.NodeMask			= pDevice->NodeMask;
		desc.NumArgumentDescs	= static_cast<uint32>(m_Arguments.size());
		desc.pArgumentDescs		= m_Arguments.data();
		desc.ByteStride			= m_Stride;
		
		return pDevice->GetDevice()->CreateCommandSignature(&desc, pRootSignature->Get(), IID_PPV_ARGS(&m_CommandSignature));
	}
	
	void D3D12CommandSignature::AddConstant(uint32 RootIndex, uint32 Count, uint32 Offset)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		argument.Constant.RootParameterIndex		= RootIndex;
		argument.Constant.Num32BitValuesToSet		= Count;
		argument.Constant.DestOffsetIn32BitValues	= Offset;

		m_Stride += Count * sizeof(uint32);

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddCBV(uint32 Slot)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		argument.ConstantBufferView.RootParameterIndex = Slot;

		m_Stride += 8;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddSRV(uint32 Slot)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argument.ShaderResourceView.RootParameterIndex = Slot;

		//m_Stride += sizeof(D3D12_SRV_;
		m_Stride += 8;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddUAV(uint32 Slot)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		argument.UnorderedAccessView.RootParameterIndex = Slot;

		m_Stride += 8;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddVertexView(uint32 Slot)
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		argument.VertexBuffer.Slot = Slot;

		m_Stride += sizeof(D3D12_VERTEX_BUFFER_VIEW);
		
		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddIndexView()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;

		m_Stride += sizeof(D3D12_INDEX_BUFFER_VIEW);

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddDraw()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

		m_Stride += sizeof(D3D12_DRAW_ARGUMENTS);

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddDrawIndexed()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		m_Stride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);

		m_Arguments.push_back(argument);
	}

	void D3D12CommandSignature::AddDispatch()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddDispatchMesh()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddDispatchRays()
	{
		D3D12_INDIRECT_ARGUMENT_DESC argument{};
		argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS;

		m_Arguments.emplace_back(argument);
	}

	void D3D12CommandSignature::AddDrawArgument(uint32 IndexCount, uint32 IndexStart, uint32 VertexStart)
	{
		m_DrawArgs.InstanceCount			= 1;
		m_DrawArgs.StartInstanceLocation	= 0;
		m_DrawArgs.IndexCountPerInstance	= IndexCount;
		m_DrawArgs.StartIndexLocation		= IndexStart;
		m_DrawArgs.BaseVertexLocation		= VertexStart;
	}

	void D3D12CommandSignature::Release()
	{
		SAFE_RELEASE(m_CommandSignature);
	}
	/**/
} // namespace lde
