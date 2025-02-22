#pragma once

#include <AgilitySDK/d3d12.h>
#include <AgilitySDK/d3d12sdklayers.h>
#include "RHI/Device.hpp"
#include <dxgi1_6.h>

#include "Core/CoreMinimal.hpp"

#include "RHI/D3D12/D3D12CommandList.hpp"
#include "RHI/D3D12/D3D12DescriptorHeap.hpp"
#include "RHI/D3D12/D3D12Fence.hpp"
#include "RHI/D3D12/D3D12Memory.hpp"
#include "RHI/D3D12/D3D12Queue.hpp"
#include "RHI/D3D12/D3D12Texture.hpp"
#include "RHI/Types.hpp"

#if DEBUG_MODE
	#include <dxgidebug.h>
#endif

namespace lde
{
	class D3D12Texture;
	class D3D12Buffer;
	class D3D12ConstantBuffer;

	struct D3D12Debug
	{
		Ref<IDXGIDebug1>		DXGIDebug;
		Ref<ID3D12Debug6>		D3DDebug;
		Ref<ID3D12DebugDevice2>	DebugDevice;
	};

	struct D3D12Features
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS	Features;  // Core
		D3D12_FEATURE_DATA_D3D12_OPTIONS1	Features1; // WaveOps
		D3D12_FEATURE_DATA_D3D12_OPTIONS3	Features3; // Barycentrics
		D3D12_FEATURE_DATA_D3D12_OPTIONS5	Features5; // Raytracing
		D3D12_FEATURE_DATA_D3D12_OPTIONS6	Features6; // Variable shading rate
		D3D12_FEATURE_DATA_D3D12_OPTIONS7	Features7; // Mesh shading / Sampler feedback
	};

	struct D3D12Capabilities
	{
		D3D_SHADER_MODEL			 HighestShaderModel;	// SM6.0 required. Desirable SM6.6.
		D3D12_RAYTRACING_TIER		 RaytracingTier;		// Desirable 1_0 at least.
		D3D12_MESH_SHADER_TIER		 MeshShaderTier;		// Wanted 1_0.
		D3D12_RESOURCE_BINDING_TIER  BindingTier;			// Wanted tier 3.
	};

	class D3D12Device : public Device
	{
	public:
		D3D12Device();
		~D3D12Device();
		
		IDXGIFactory7* GetFactory() { return m_Factory.Get(); }
		IDXGIAdapter4* GetAdapter() { return m_Adapter.Get(); }
		ID3D12Device8* GetDevice()  { return m_Device.Get();  }
		
		// Since running single GPU, node stays as 0.
		uint32 NodeMask = 0;

		// Default; Graphics Queue
		//void WaitForGPU();

		/**
		 * @brief Wait for GPU to finish it's work on given type of a Queue.
		 * @param eType Queue to wait for.
		 */
		void WaitForGPU(CommandType eType);

		void FlushGPU();
		void IdleGPU();

		/**
		 * @brief Execute CommandList of a given type
		 * @param eType 
		 * @param bResetAllocator 
		 */
		void ExecuteCommandList(CommandType eType, bool bResetAllocator = false);

		void ExecuteAllCommandLists(bool bResetAllocators = false);

		//D3D12Fence* GetFence() { return m_Fence.get(); }
		
		// One per frame buffer
		struct FrameResources
		{
			D3D12CommandList*		GraphicsCommandList;

			// TODO:
			D3D12CommandList*		ComputeCommandList;
			//D3D12CommandList*		UploadCommandList;
		} m_FrameResources[FRAME_COUNT];

		D3D12Queue* GraphicsQueue;
		D3D12Queue* ComputeQueue;

		void				CreateFrameResources();
		FrameResources&		GetFrameResources()		{ return m_FrameResources[FRAME_INDEX]; }

		D3D12Queue*			 GetGfxQueue()			{ return GraphicsQueue; }
		D3D12Queue*			 GetComputeQueue()		{ return ComputeQueue;  }

		D3D12CommandList*	 GetGfxCommandList()	{ return m_FrameResources[FRAME_INDEX].GraphicsCommandList; }
		D3D12CommandList*	 GetComputeCommandList(){ return m_FrameResources[FRAME_INDEX].ComputeCommandList; }

		D3D12DescriptorHeap* GetShaderResourceHeap()	{ return m_ShaderResourceHeap.get(); }
		D3D12DescriptorHeap* GetRenderTargetHeap()		{ return m_RenderTargetHeap.get(); }
		D3D12DescriptorHeap* GetDepthStencilHeap()		{ return m_DepthStencilHeap.get(); }

		/**
		 * @brief				Allocate given Descriptor from the Heap of given enum type.
		 * @param eType			Type of a Heap to allocate from.
		 * @param Descriptor	Target Descriptor.
		 * @param Count			How much space Descriptor needs. Defaults to 1.
		 */
		void Allocate(HeapType eType, D3D12Descriptor& Descriptor, uint32 Count = 1);
		
		D3D12Features		Features;
		D3D12Capabilities	Capabilities;

	private:
		Ref<IDXGIFactory7> m_Factory;
		Ref<IDXGIAdapter4> m_Adapter;
		Ref<ID3D12Device8> m_Device;

		DXGI_ADAPTER_DESC3 m_AdapterDesc{};

		//std::unique_ptr<D3D12Fence> m_Fence;

		std::unique_ptr<D3D12DescriptorHeap> m_ShaderResourceHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_RenderTargetHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_DepthStencilHeap;

	private:
		void Create();
		void Release();

		void CreateAdapter();
		void CreateDevice();

		void QueryDeviceFeatures();
		
		void CreateDescriptorHeaps();

#if DEBUG_MODE
		D3D12Debug m_DebugDevices;
#endif

	public:
		std::vector<D3D12Buffer*>			Buffers;
		std::vector<D3D12ConstantBuffer*>	ConstantBuffers;
		std::vector<D3D12Texture*>			Textures;

		D3D12Buffer*			GetBuffer(uint32 Index);
		D3D12ConstantBuffer*	GetConstantBuffer(uint32 Index);
		D3D12Texture*			GetTexture(uint32 Index);


		void CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Mips, uint32 Count);
		void CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 MipSlice, uint32 Count);
		void CreateRTV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format);
		void CreateDSV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);


		/* ======================== RHI implementations ======================== */

		BufferHandle	CreateBuffer(BufferDesc Desc) override final;
		BufferHandle	CreateConstantBuffer(void* pData, usize Size) override final;
		// Temporal
		TextureHandle	CreateTexture(D3D12Texture* pTexture);
		TextureHandle	CreateTexture(TextureDesc Desc);

		void			DestroyBuffer(BufferHandle Handle);
		void			DestroyConstantBuffer(BufferHandle Handle);
		void			DestroyTexture(TextureHandle Handle);

		
		
	private:

	};
} // namespace lde
