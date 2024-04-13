#pragma once

#include <AgilitySDK/d3d12.h>
#include <AgilitySDK/d3d12sdklayers.h>
#include <RHI/Device.hpp>
#include <dxgi1_6.h>

#include <Core/CoreMinimal.hpp>

#include <RHI/D3D12/D3D12CommandList.hpp>
#include <RHI/D3D12/D3D12DescriptorHeap.hpp>
#include <RHI/D3D12/D3D12Fence.hpp>
#include <RHI/D3D12/D3D12Memory.hpp>
#include <RHI/D3D12/D3D12Queue.hpp>
#include <RHI/Types.hpp>
#include <unordered_map>

#if DEBUG_MODE
	#include <dxgidebug.h>
#endif

namespace lde::RHI
{
	// Since running single GPU, node stays as 0.
	constexpr uint32 DEVICE_NODE = 0;

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
		D3D12Device()  { Create();  }
		~D3D12Device() { Release(); }

		D3D12Features		Features;
		D3D12Capabilities	Capabilities;
		
		IDXGIFactory7* GetFactory() { return m_Factory.Get(); }
		IDXGIAdapter4* GetAdapter() { return m_Adapter.Get(); }
		ID3D12Device8* GetDevice()  { return m_Device.Get();  }
		
		// Default; Graphics Queue
		void WaitForGPU();

		/**
		 * @brief Wait for GPU to finish it's work on given type of a Queue.
		 * @param eType Queue to wait for.
		 */
		void WaitForGPU(CommandType eType);

		/**
		 * @brief Wait for all types of queues to finish it's work.
		 */
		void WaitForAllQueues();

		void FlushGPU();
		void IdleGPU();

		/**
		 * @brief Execute CommandList of a given type
		 * @param eType 
		 * @param bResetAllocator 
		 */
		void ExecuteCommandList(CommandType eType, bool bResetAllocator = false);
		void ExecuteAllCommandLists(bool bResetAllocators);

		D3D12Fence* GetFence() { return m_Fence.get(); }

		/**
		 * @brief 
		 */
		struct FrameResources
		{
			D3D12CommandList*	GraphicsCommandList;
			D3D12Queue*			GraphicsQueue;
			uint64				FrameFenceValue = 0;

			D3D12CommandList*	ComputeCommandList;
			D3D12Queue*			ComputeQueue;
			uint64				ComputeFenceValue = 0;

			D3D12CommandList*	UploadCommandList;
			D3D12Queue*			UploadQueue;
			uint64				UploadFenceValue = 0;
		} m_FrameResources{};

		void			CreateFrameResources();
		FrameResources& GetFrameResources()			{ return m_FrameResources; }

		D3D12Queue*			GetGfxQueue()			{ return m_FrameResources.GraphicsQueue;		}
		D3D12CommandList*	GetGfxCommandList()		{ return m_FrameResources.GraphicsCommandList;	}

		D3D12DescriptorHeap* GetSRVHeap()			{ return m_SRVHeap.get();		}
		D3D12DescriptorHeap* GetDSVHeap()			{ return m_DSVHeap.get();		}
		D3D12DescriptorHeap* GetRTVHeap()			{ return m_RTVHeap.get();		}


		/**
		 * @brief Allocate given Descriptor from the Heap of given enum type.
		 * @param eType 
		 * @param Descriptor 
		 * @param Count 
		 */
		void Allocate(HeapType eType, D3D12Descriptor& Descriptor, uint32 Count = 1);
		
	private:
		Ref<IDXGIFactory7> m_Factory;
		Ref<IDXGIAdapter4> m_Adapter;
		Ref<ID3D12Device8> m_Device;

		DXGI_ADAPTER_DESC3 m_AdapterDesc{};

		std::unique_ptr<D3D12Fence> m_Fence;

		std::unique_ptr<D3D12DescriptorHeap> m_SRVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_DSVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_RTVHeap;


	private:
		void Create();
		void Release();

		void CreateAdapter();
		void CreateDevice();

		/**
		 * @brief Check if SM6.6 is supported.
		 */
		void QueryShaderModel();

		/**
		 * @brief  Gather Device features
		 */
		void QueryFeatures();

		void CreateQueues();
		void CreateCommandLists();
		void CreateHeaps();

#if DEBUG_MODE
		D3D12Debug m_DebugDevices;
#endif

	public:
		// TODO:
		std::vector<D3D12Buffer*>			Buffers;
		std::vector<D3D12ConstantBuffer*>	ConstantBuffers;
		std::vector<D3D12Texture*>			Textures;

		/* ======================== RHI implementations ======================== */

		BufferHandle	CreateBuffer(BufferDesc Desc) override final;
		BufferHandle	CreateConstantBuffer(void* pData, usize Size) override final;
		// Temporal
		TextureHandle	CreateTexture(D3D12Texture* pTexture);
		//TextureHandle	CreateTexture(TextureDesc Desc) override final;

		void			DestroyBuffer(BufferHandle Handle);
		void			DestroyConstantBuffer(BufferHandle Handle);
		void			DestroyTexture(TextureHandle Handle);


		void CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Mips, uint32 Count);
		void CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 MipSlice, uint32 Count);

	private:

	};
} // namespace lde::RHI
