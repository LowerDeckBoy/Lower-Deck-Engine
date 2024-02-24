#pragma once

/*
	RHI/D3D12/D3D12RHI.hpp

*/

#include <RHI/RHI.hpp>
#include <RHI/RHICommon.hpp>
#include <RHI/Types.hpp>

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Device.hpp"

#include "D3D12CommandList.hpp"
#include "D3D12DepthBuffer.hpp"
#include "D3D12Fence.hpp"
#include "D3D12Memory.hpp"
#include "D3D12PipelineState.hpp"
#include "D3D12Queue.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12SwapChain.hpp"
#include "D3D12Utility.hpp"
#include "D3D12Viewport.hpp"


namespace lde::RHI
{
	class D3D12Buffer;
	class D3D12IndexBuffer;
	class D3D12ConstantBuffer;

	class D3D12RHI : public RHI
	{
	public:
		D3D12RHI();
		~D3D12RHI();

		
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
		SwapChain* GetSwapChain() override { return ((D3D12SwapChain*)SwapChain.get()); }

		//CommandList* GetGfxCommandList() override { return (D3D12CommandList*)Device->GetGfxCommandList()->Get(); }
		
		void Initialize();
		void Release();

		void OnResize(uint32 Width, uint32 Height);

		std::unique_ptr<D3D12Device>	Device;
		std::unique_ptr<D3D12SwapChain> SwapChain;
		
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

		void BindIndexBuffer(Buffer* pIndexBuffer) const;
		/* For non-bindless only */
		void BindVertexBuffers(std::span<D3D12Buffer*> pIndexBuffers, uint32 StartSlot) const;
		void BindConstantBuffer(ConstantBuffer* pConstBuffer, uint32 Slot);

		void Draw(uint32 VertexCount) const;
		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const;
		void DrawIndexedInstanced(uint32 InstanceCount, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) const;


	private:
		
	};
}
