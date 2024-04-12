#include "D3D12Buffer.hpp"
#include "D3D12RHI.hpp"

#include <Core/Logger.hpp>
#include <Platform/Window.hpp>

namespace lde::RHI
{
	D3D12RHI::D3D12RHI()
	{
		Initialize();
	}

	D3D12RHI::~D3D12RHI()
	{
		Release();
	}

	void D3D12RHI::BeginFrame()
	{
		OpenList(Device->GetGfxCommandList());
		//D3D12Memory::SetFrameIndex(FRAME_INDEX);
		
		TransitResource(SwapChain->GetBackbuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		SetViewport();

		Device->GetGfxCommandList()->Get()->SetDescriptorHeaps(1, Device->GetSRVHeap()->GetAddressOf());
	}

	void D3D12RHI::RecordCommandLists()
	{
	}

	void D3D12RHI::Update()
	{
	}

	void D3D12RHI::Render()
	{
	}

	void D3D12RHI::EndFrame()
	{
		
	}

	void D3D12RHI::Present(bool bVSync)
	{
		TransitResource(SwapChain->GetBackbuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		Device->ExecuteCommandList(CommandType::eGraphics, false);

		DX_CALL(SwapChain->Get()->Present((bVSync ? 1 : 0), 0));

		MoveToNextFrame();
	}

	void D3D12RHI::Initialize()
	{
		Device = std::make_unique<D3D12Device>();

		D3D12MA::ALLOCATOR_DESC desc{};
		desc.pAdapter = Device->GetAdapter();
		desc.pDevice  = Device->GetDevice();

		DX_CALL(D3D12MA::CreateAllocator(&desc, &D3D12Memory::Allocator));

		SwapChain = std::make_unique<D3D12SwapChain>(Device.get(), Device->GetGfxQueue(), lde::Window::Width, lde::Window::Height);

		SceneViewport = new D3D12Viewport(lde::Window::Width, lde::Window::Height);

		SceneDepth = new D3D12DepthBuffer(Device.get(), Device->GetDSVHeap(), SceneViewport);

		LOG_INFO("Backend: D3D12 initialized.");

	}

	void D3D12RHI::Release()
	{
		delete SceneDepth;
		SwapChain.reset();;
		SAFE_RELEASE(D3D12Memory::Allocator);
		Device.reset();
		LOG_INFO("Backend: D3D12 released.");
	}

	uint32 D3D12RHI::QueryAdapterMemory()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo{};
		Device->GetAdapter()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

		return static_cast<uint32>(memoryInfo.CurrentUsage / 1024 / 1024);
	}

	void D3D12RHI::MoveToNextFrame()
	{
		const uint64 currentFenceValue = Device->GetFence()->GetValue();

		Device->GetFence()->Signal(Device->GetGfxQueue(), Device->GetFence()->GetValue());

		FRAME_INDEX = SwapChain->Get()->GetCurrentBackBufferIndex();

		if (Device->GetFence()->IsValueCompleted(Device->GetFence()->GetValue()))
		{
			Device->GetFence()->SetEvent();
			Device->GetFence()->Wait();
		}

		Device->GetFence()->UpdateValue(static_cast<uint32>(currentFenceValue));
	}

	void D3D12RHI::OpenList(D3D12CommandList* pCommandList)
	{
		pCommandList->Reset();
	}
	
	void D3D12RHI::OnResize(uint32 Width, uint32 Height)
	{
		Device->WaitForGPU();

		Device->GetGfxCommandList()->Reset();
		
		Device->GetFence()->OnResize();
		SceneViewport->Set(Width, Height);
		SwapChain->OnResize(Width, Height);
		SceneDepth->OnResize(Device->GetDSVHeap(), SceneViewport);

		Device->ExecuteCommandList(CommandType::eGraphics, false);

		Device->IdleGPU();
	}

	void D3D12RHI::SetMainRenderTarget() const
	{
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			SwapChain->RTVHeap()->CpuStartHandle(),
			FRAME_INDEX,
			SwapChain->RTVHeap()->GetDescriptorSize());
		const auto& depthHandle{ SceneDepth->DSV().GetCpuHandle() };
		Device->GetGfxCommandList()->Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);
	}

	void D3D12RHI::ClearMainRenderTarget() const
	{
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			SwapChain->RTVHeap()->CpuStartHandle(),
			FRAME_INDEX,
			SwapChain->RTVHeap()->GetDescriptorSize());
		Device->GetGfxCommandList()->Get()->ClearRenderTargetView(rtvHandle, ClearColor.data(), 0, nullptr);
	}

	void D3D12RHI::ClearDepthStencil()
	{
		auto& depthHandle = SceneDepth->DSV().GetCpuHandle();
		Device->GetGfxCommandList()->Get()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);
	}

	void D3D12RHI::SetViewport() const
	{
		auto viewport = SceneViewport->GetViewport();
		auto scissor  = SceneViewport->GetScissor();
		Device->GetGfxCommandList()->Get()->RSSetViewports(1, &viewport);
		Device->GetGfxCommandList()->Get()->RSSetScissorRects(1, &scissor);
	}

	void D3D12RHI::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE* DepthCpuHandle)
	{
		Device->GetGfxCommandList()->Get()->OMSetRenderTargets(1, &RtvCpuHandle, FALSE, DepthCpuHandle);
	}

	void D3D12RHI::SetRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& RtvCpuHandles, D3D12_CPU_DESCRIPTOR_HANDLE DepthCpuHandle)
	{
		Device->GetGfxCommandList()->Get()->OMSetRenderTargets(static_cast<uint32>(RtvCpuHandles.size()), RtvCpuHandles.data(), TRUE, &DepthCpuHandle);
	}

	void D3D12RHI::ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle)
	{
		Device->GetGfxCommandList()->Get()->ClearRenderTargetView(RtvCpuHandle, ClearColor.data(), 0, nullptr);
	}

	void D3D12RHI::SetRootSignature(D3D12RootSignature* pRootSignature) const
	{
		if (pRootSignature->Type == PipelineType::eGraphics)
		{
			Device->GetGfxCommandList()->Get()->SetGraphicsRootSignature(pRootSignature->Get());
		}
		else
		{
			Device->GetGfxCommandList()->Get()->SetComputeRootSignature(pRootSignature->Get());
		}
	}

	void D3D12RHI::SetRootSignature(ID3D12RootSignature* pRootSignature) const
	{
		Device->GetGfxCommandList()->Get()->SetGraphicsRootSignature(pRootSignature);
	}

	void D3D12RHI::SetPipeline(D3D12PipelineState* pPipelineState) const
	{
		Device->GetGfxCommandList()->Get()->SetPipelineState(pPipelineState->Get());
	}

	void D3D12RHI::TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = Before;
		barrier.Transition.StateAfter = After;

		Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &barrier);
	}

	void D3D12RHI::TransitResource(Ref<ID3D12Resource> pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pResource.Get();
		barrier.Transition.StateBefore = Before;
		barrier.Transition.StateAfter = After;

		Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &barrier);
	}

	void D3D12RHI::UploadResource(ID3D12Resource* pDst, ID3D12Resource* pSrc, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(Device->GetGfxCommandList()->Get(), pDst, pSrc, 0, 0, 1, &Subresource);
	}

	void D3D12RHI::UploadResource(ID3D12Resource** ppDst, ID3D12Resource** ppSrc, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(Device->GetGfxCommandList()->Get(), (*ppDst), (*ppSrc), 0, 0, 1, &Subresource);
	}

	void D3D12RHI::UploadResource(Ref<ID3D12Resource> ppDst, Ref<ID3D12Resource> ppSrc, D3D12_SUBRESOURCE_DATA Subresource)
	{
		::UpdateSubresources(Device->GetGfxCommandList()->Get(), ppDst.Get(), ppSrc.Get(), 0, 0, 1, &Subresource);
	}

	void D3D12RHI::CopyResource(Ref<ID3D12Resource> ppDst, Ref<ID3D12Resource> ppSrc)
	{
		Device->GetGfxCommandList()->Get()->CopyResource(ppDst.Get(), ppSrc.Get());
	}

	void D3D12RHI::BindIndexBuffer(Buffer* pIndexBuffer) const
	{
		auto view = GetIndexView((D3D12Buffer*)pIndexBuffer);
		Device->GetGfxCommandList()->Get()->IASetIndexBuffer(&view);
	}

	void D3D12RHI::BindVertexBuffers(std::span<D3D12Buffer*> pIndexBuffers, uint32 StartSlot) const
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
		for (auto& buffer : pIndexBuffers)
		{
			views.emplace_back(buffer->Get()->GetGPUVirtualAddress(), static_cast<uint32>(buffer->GetDesc().Size, buffer->GetDesc().Stride));
		}

		Device->GetGfxCommandList()->Get()->IASetVertexBuffers(StartSlot, static_cast<uint32>(views.size()), views.data());
	}

	void D3D12RHI::Draw(uint32 VertexCount) const
	{
		Device->GetGfxCommandList()->Draw(VertexCount);
	}

	void D3D12RHI::DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const
	{
		Device->GetGfxCommandList()->DrawIndexedInstanced(1, IndexCount, BaseIndex, BaseVertex);
	}

	void D3D12RHI::DrawIndexedInstanced(uint32 InstanceCount, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const
	{
		Device->GetGfxCommandList()->DrawIndexedInstanced(IndexCount, InstanceCount, BaseIndex, BaseVertex);
	}

	void D3D12RHI::BindConstantBuffer(ConstantBuffer* pConstBuffer, uint32 Slot)
	{
		Device->GetGfxCommandList()->BindConstantBuffer(Slot, ((D3D12ConstantBuffer*)pConstBuffer));
	}

}
