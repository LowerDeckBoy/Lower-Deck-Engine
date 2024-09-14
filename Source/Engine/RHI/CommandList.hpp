#pragma once

#include <RHI/Types.hpp>
#include <span>

namespace lde
{
	class Buffer;
	class ConstantBuffer;
	class PipelineState;

	class CommandList
	{
	public:

		//virtual void Reset() = 0;

		// Draw single instance
		virtual void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) = 0;
		// Draw multiple instances
		virtual void DrawIndexedInstanced(uint32 Instances, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) = 0;
		// Draw vertices
		virtual void Draw(uint32 VertexCount) = 0;

		//virtual void DrawIndirect(uint32 IndexCount, uint32 VertexCount) = 0;

		virtual void BindVertexBuffer(Buffer* pBuffer) = 0;
		virtual void BindIndexBuffer(Buffer* pBuffer) = 0;
		virtual void BindConstantBuffer(uint32 Slot, ConstantBuffer* pBuffer) = 0;

		virtual void PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset = 0) = 0;

		//void ResourceBarrier(Ref<ID3D12Resource> ppResource, ResourceState Before, ResourceState After);
		//void UploadResource(Ref<ID3D12Resource> ppSrc, Ref<ID3D12Resource> ppDst, d3d12Subresource);
		// TODOs:
		// 
		//virtual void SetRenderTarget(uint64* pHandle) = 0;
		//virtual void SetRenderTargets(std::span<uint64*> pHandles) = 0;
		//virtual void ClearRenderTarget(uint64* pHandle) = 0;
		//virtual void ClearDepthStencil(uint64* pHandle) = 0;

		CommandType& GetType() { return m_Type; }

	protected:
		CommandType m_Type{};

	};
} // namespace lde
