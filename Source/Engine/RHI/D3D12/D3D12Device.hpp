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
#	include <dxgidebug.h>
#endif

namespace lde::RHI
{
	class D3D12RootSignature;

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
		void WaitForGPU(CommandType eType);
		void FlushGPU();
		void IdleGPU();

		/**
		 * @brief Execute CommandList of a given type
		 * @param eType 
		 * @param bResetAllocator 
		 */
		void ExecuteCommandList(CommandType eType, bool bResetAllocator = false);

		D3D12Fence* GetFence()						{ return m_Fence.get(); }

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

		FrameResources& GetFrameResources() { return m_FrameResources; }

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

		std::unique_ptr<D3D12Fence> m_Fence;

		//std::unique_ptr<D3D12Queue> m_GfxQueue;
		//std::unique_ptr<D3D12Queue> m_ComputeQueue;
		//std::unique_ptr<D3D12Queue> m_UploadQueue;

		//std::unique_ptr<D3D12CommandList> m_GfxCommandList;
		//std::unique_ptr<D3D12CommandList> m_ComputeCommandList;
		//std::unique_ptr<D3D12CommandList> m_UploadCommandList;

		std::unique_ptr<D3D12DescriptorHeap> m_SRVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_DSVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_RTVHeap;

		

		void CreateFrameResources();

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

		DXGI_ADAPTER_DESC3 m_AdapterDesc{};
		
#if DEBUG_MODE
		D3D12Debug m_DebugDevices;
#endif

	public:


		//std::vector<D3D12Buffer*>	Buffers;
		//std::vector<D3D12Texture*>	Textures;

		/* ======================== RHI implementations ======================== */

		Buffer*			CreateBuffer(BufferDesc Desc) override final;
		ConstantBuffer* CreateConstantBuffer(void* pData, usize Size) override final;
		Texture*		CreateTexture(TextureDesc Desc) override final;

		void CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count = 1);
		void CreateSRV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Mips, uint32 Count);
		void CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 Count = 1);
		void CreateUAV(ID3D12Resource* pResource, D3D12Descriptor& Descriptor, uint32 MipSlice, uint32 Count = 1);

	private:

	};
} // namespace lde::RHI
