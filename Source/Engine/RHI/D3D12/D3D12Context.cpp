#include "D3D12Buffer.hpp"
#include "D3D12Context.hpp"

#include <Platform/Window.hpp>
#include <Core/Logger.hpp>

namespace lde::RHI
{
	D3D12Context::D3D12Context()
	{
		Initialize();
	}

	D3D12Context::~D3D12Context()
	{
		Release();
	}

	void D3D12Context::BeginFrame()
	{
		OpenList(GraphicsCommandList);

		TransitResource(SwapChain->GetBackbuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		SetViewport();
	}

	void D3D12Context::RecordCommandLists()
	{
	}

	void D3D12Context::Update()
	{
	}

	void D3D12Context::Render()
	{
	}

	void D3D12Context::EndFrame()
	{
		
	}

	void D3D12Context::Present(bool bVSync)
	{
		TransitResource(SwapChain->GetBackbuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		ExecuteCommandList(GraphicsCommandList, Device->GetGfxQueue(), false);

		DX_CALL(SwapChain->Get()->Present((bVSync ? 1 : 0), 0));

		MoveToNextFrame();
	}

	void D3D12Context::Initialize()
	{
		Device = std::make_unique<D3D12Device>();

		D3D12MA::ALLOCATOR_DESC desc{};
		desc.pAdapter = Device->GetAdapter();
		desc.pDevice = Device->GetDevice();

		DX_CALL(D3D12MA::CreateAllocator(&desc, &D3D12Memory::Allocator));

		SwapChain = std::make_unique<D3D12SwapChain>(Device.get(), Device->GetGfxQueue(), lde::Window::Width, lde::Window::Height);

		GraphicsCommandList = new D3D12CommandList(Device.get(), CommandType::eGraphics, "Graphics Command List");

		Heap = new D3D12DescriptorHeap(Device.get(), HeapType::eSRV, 16384, L"CPU Heap");
		StagingHeap = new D3D12DescriptorHeap(Device.get(), HeapType::eSRV, 16384, L"GPU Heap");

		RenderTargetHeap = new D3D12DescriptorHeap(Device.get(), HeapType::eRTV, 128, L"Render Target Heap");
		DepthHeap = new D3D12DescriptorHeap(Device.get(), HeapType::eDSV, 64, L"Depth Heap");

		MipMapHeap = new D3D12DescriptorHeap(Device.get(), HeapType::eSRV, 512, L"MipMap Heap");

		SceneViewport = new D3D12Viewport(lde::Window::Width, lde::Window::Height);

		SceneDepth = new D3D12DepthBuffer(Device.get(), DepthHeap, SceneViewport);

		// TODO:
		//GlobalRootSignature.Create(Adapter, {}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, L"Global HLSL Root Signature");

		LOG_INFO("Backend: D3D12 initialized.");

	}

	void D3D12Context::Release()
	{
		delete SceneDepth;
		delete Heap;
		delete StagingHeap;
		delete RenderTargetHeap;
		delete DepthHeap;
		delete MipMapHeap;
		delete GraphicsCommandList;
		SwapChain.reset();;
		SAFE_RELEASE(D3D12Memory::Allocator);
		Device.reset();
		LOG_INFO("Backend: D3D12 released.");
	}

	uint32 D3D12Context::QueryAdapterMemory()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo{};
		Device->GetAdapter()->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

		return static_cast<uint32>(memoryInfo.CurrentUsage / 1024 / 1024);
	}

	void D3D12Context::MoveToNextFrame()
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

	void D3D12Context::OpenList(D3D12CommandList* pCommandList)
	{
		pCommandList->Reset();
	}
	
	void D3D12Context::ExecuteCommandList(D3D12CommandList* pCommandList, D3D12Queue* pCommandQueue, bool bResetAllocators)
	{
		DX_CALL(pCommandList->Get()->Close());

		std::array<ID3D12CommandList*, 1> commandLists{ pCommandList->Get() };

		pCommandQueue->Get()->ExecuteCommandLists(static_cast<uint32>(commandLists.size()), commandLists.data());
		
		if (bResetAllocators)
		{
			pCommandList->ResetList();
		}

		Device->WaitForGPU();
	}
	
	void D3D12Context::OnResize(uint32 Width, uint32 Height)
	{
		Device->WaitForGPU();

		GraphicsCommandList->Reset();
		
		Device->GetFence()->OnResize();
		SceneViewport->Set(Width, Height);
		SwapChain->OnResize(Width, Height);
		SceneDepth->OnResize(DepthHeap, SceneViewport);

		ExecuteCommandList(GraphicsCommandList, Device->GetGfxQueue(), false);

		Device->IdleGPU();
	}

	void D3D12Context::SetRenderTarget()
	{
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			SwapChain->RTVHeap()->CpuStartHandle(),
			FRAME_INDEX,
			SwapChain->RTVHeap()->GetDescriptorSize());
		const auto depthHandle{ SceneDepth->DSV().GetCpuHandle() };
		GraphicsCommandList->Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, &depthHandle);
	}

	void D3D12Context::ClearRenderTarget()
	{
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			SwapChain->RTVHeap()->CpuStartHandle(),
			FRAME_INDEX,
			SwapChain->RTVHeap()->GetDescriptorSize());
		GraphicsCommandList->Get()->ClearRenderTargetView(rtvHandle, ClearColor.data(), 0, nullptr);
	}

	void D3D12Context::ClearDepthStencil()
	{
		auto depthHandle{ SceneDepth->DSV().GetCpuHandle() };
		GraphicsCommandList->Get()->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, D3D12_MAX_DEPTH, 0, 0, nullptr);
	}

	void D3D12Context::SetViewport()
	{
		auto viewport = SceneViewport->GetViewport();
		auto scissor  = SceneViewport->GetScissor();
		GraphicsCommandList->Get()->RSSetViewports(1, &viewport);
		GraphicsCommandList->Get()->RSSetScissorRects(1, &scissor);
	}

	void D3D12Context::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE DepthCpuHandle)
	{
		GraphicsCommandList->Get()->OMSetRenderTargets(1, &RtvCpuHandle, FALSE, &DepthCpuHandle);
	}

	void D3D12Context::SetRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& RtvCpuHandles, D3D12_CPU_DESCRIPTOR_HANDLE DepthCpuHandle)
	{
		GraphicsCommandList->Get()->OMSetRenderTargets(static_cast<uint32>(RtvCpuHandles.size()), RtvCpuHandles.data(), TRUE, &DepthCpuHandle);
	}

	void D3D12Context::ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle)
	{
		GraphicsCommandList->Get()->ClearRenderTargetView(RtvCpuHandle, ClearColor.data(), 0, nullptr);
	}

	void D3D12Context::SetRootSignature(D3D12RootSignature* pRootSignature) const
	{
		if (pRootSignature->Type == PipelineType::eGraphics)
		{
			GraphicsCommandList->Get()->SetGraphicsRootSignature(pRootSignature->GetRootSignature());
		}
		else
		{
			GraphicsCommandList->Get()->SetComputeRootSignature(pRootSignature->GetRootSignature());
		}
	}

	void D3D12Context::SetPipeline(D3D12PipelineState* pPipelineState) const
	{
		GraphicsCommandList->Get()->SetPipelineState(pPipelineState->Get());
	}

	void D3D12Context::TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
	{
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pResource;
		barrier.Transition.StateBefore = Before;
		barrier.Transition.StateAfter = After;

		GraphicsCommandList->Get()->ResourceBarrier(1, &barrier);
	}

	void D3D12Context::UploadResource(ID3D12Resource* pDst, ID3D12Resource* pSrc, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(GraphicsCommandList->Get(), pDst, pSrc, 0, 0, 1, &Subresource);
	}

	void D3D12Context::UploadResource(ID3D12Resource** ppDst, ID3D12Resource** ppSrc, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(GraphicsCommandList->Get(), (*ppDst), (*ppSrc), 0, 0, 1, &Subresource);

	}

	void D3D12Context::UploadResource(Ref<ID3D12Resource> ppDst, Ref<ID3D12Resource> ppSrc, D3D12_SUBRESOURCE_DATA& Subresource)
	{
		::UpdateSubresources(GraphicsCommandList->Get(), ppDst.Get(), ppSrc.Get(), 0, 0, 1, &Subresource);
	}

	void D3D12Context::BindIndexBuffer(D3D12Buffer* pIndexBuffer) const
	{
		auto view = GetIndexView(pIndexBuffer);
		GraphicsCommandList->Get()->IASetIndexBuffer(&view);
	}

	void D3D12Context::BindVertexBuffers(std::span<D3D12Buffer*> pIndexBuffers, uint32 StartSlot) const
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
		for (auto& buffer : pIndexBuffers)
		{
			views.emplace_back(buffer->Get()->GetGPUVirtualAddress(), static_cast<uint32>(buffer->GetDesc().Size, buffer->GetDesc().Stride));
		}

		GraphicsCommandList->Get()->IASetVertexBuffers(StartSlot, static_cast<uint32>(views.size()), views.data());
	}

	void D3D12Context::DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const
	{
		GraphicsCommandList->Get()->DrawIndexedInstanced(IndexCount, 1, BaseIndex, BaseVertex, 0);
	}

	void D3D12Context::DrawIndexedInstanced(uint32 InstanceCount, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const
	{
		GraphicsCommandList->Get()->DrawIndexedInstanced(IndexCount, InstanceCount, BaseIndex, BaseVertex, 0);
	}

	void D3D12Context::BindConstantBuffer(D3D12ConstantBuffer* pConstBuffer, uint32 Slot)
	{
		GraphicsCommandList->Get()->SetGraphicsRootConstantBufferView(Slot, pConstBuffer->GetBuffer()->GetGPUVirtualAddress());
	}

}
