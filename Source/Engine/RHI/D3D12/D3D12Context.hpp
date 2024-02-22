#pragma once

/*
	RHI/D3D12/D3D12Context.hpp

*/

#include <RHI/RHI.hpp>

#include "D3D12Utility.hpp"
#include "D3D12Device.hpp"
#include "D3D12Memory.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Queue.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Fence.hpp"
#include "D3D12DescriptorHeap.hpp"
#include "D3D12DepthBuffer.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12PipelineState.hpp"

#include "RHI/RHICommon.hpp"
#include "RHI/Types.hpp"

namespace lde::RHI
{
	class Window;
	class D3D12Buffer;
	class D3D12IndexBuffer;
	class D3D12ConstantBuffer; //template<typename T> 

	class D3D12Context : public RHI
	{
	public:
		D3D12Context();
		~D3D12Context();

		
		void BeginFrame() override;
		void RecordCommandLists() override;
		void Update() override;
		void Render() override;
		void EndFrame() override;
		void Present(bool bVSync) override;

		/**
		 * @brief 
		 * @return RHI specific Device
		 */
		Device*	GetDevice() override { return this->Device.get(); }
		/**
		 * @brief 
		 * @return RHI specific SwapChain
		 */
		SwapChain* GetSwapChain() override { return this->SwapChain.get(); }

		void Initialize();
		void Release();

		void OnResize(uint32 Width, uint32 Height);

		std::unique_ptr<D3D12Device>	Device;
		std::unique_ptr<D3D12SwapChain> SwapChain;

		D3D12CommandList* GraphicsCommandList = nullptr;

		/// @brief CPU visible
		D3D12DescriptorHeap* Heap = nullptr;
		/// @brief GPU visible
		D3D12DescriptorHeap* StagingHeap = nullptr;
		/// @brief For allocating Render Targets, GBuffers etc.
		D3D12DescriptorHeap* RenderTargetHeap = nullptr;
		/// @brief For allocating Depth Views
		D3D12DescriptorHeap* DepthHeap = nullptr;
		/// @brief Used only to create MipMap UAVs; avoids polluting main SRV heap.
		D3D12DescriptorHeap* MipMapHeap = nullptr;

		/// @brief Scene viewport
		D3D12Viewport* SceneViewport = nullptr;

		/// @brief 
		D3D12DepthBuffer* SceneDepth = nullptr;

		/// @brief For HLSL RootSignature
		inline static D3D12RootSignature GlobalRootSignature;

		uint32 QueryAdapterMemory();

		void MoveToNextFrame();

		void OpenList(D3D12CommandList* pCommandList);
		//void CloseList(D3D12CommandList* pCommandList);

		void ExecuteCommandList(D3D12CommandList* pCommandList, D3D12Queue* pCommandQueue, bool bResetAllocators = false);

		//void ExecuteUploadList(bool bResetAllocators = false);
		//void ExecuteGraphicsList(D3D12CommandList* pCommandList, D3D12Queue* pCommandQueue, bool bResetAllocators = false);

		// Set current Swapchain backbuffer
		void SetRenderTarget();
		void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE DepthCpuHandle);
		void SetRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& RtvCpuHandles, D3D12_CPU_DESCRIPTOR_HANDLE DepthCpuHandle);
		// Clear current Swapchain backbuffer
		void ClearRenderTarget(); 
		void ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuHandle);
		void ClearDepthStencil();

		// Bind scene Viewport and Scissor
		void SetViewport();

		void SetRootSignature(D3D12RootSignature* pRootSignature) const;
		void SetPipeline(D3D12PipelineState* pState) const;

		void TransitResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);

		void UploadResource(ID3D12Resource* pDst, ID3D12Resource* pSrc, D3D12_SUBRESOURCE_DATA& Subresource);
		void UploadResource(ID3D12Resource** ppDst, ID3D12Resource** ppSrc, D3D12_SUBRESOURCE_DATA& Subresource);
		void UploadResource(Ref<ID3D12Resource> ppDst, Ref<ID3D12Resource> ppSrc, D3D12_SUBRESOURCE_DATA& Subresource);

		void BindIndexBuffer(D3D12Buffer* pIndexBuffer) const;
		/* For non-bindless only */
		void BindVertexBuffers(std::span<D3D12Buffer*> pIndexBuffers, uint32 StartSlot) const;
		void BindConstantBuffer(D3D12ConstantBuffer* pConstBuffer, uint32 Slot);

		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const;
		void DrawIndexedInstanced(uint32 InstanceCount, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const;


	private:
		
	};
}
