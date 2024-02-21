#pragma once

#include <RHI/Device.hpp>
#include <AgilitySDK/d3d12.h>
#include <AgilitySDK/d3d12sdklayers.h>
#include <dxgi1_6.h>

#include <Core/CoreMinimal.hpp>

#include <RHI/Types.hpp>
#include <RHI/D3D12/D3D12Fence.hpp>
#include <RHI/D3D12/D3D12Queue.hpp>
#include <RHI/D3D12/D3D12DescriptorHeap.hpp>

#if DEBUG_MODE
#	include <dxgidebug.h>
#endif

namespace mf::RHI
{
	
	struct D3D12Debug
	{
		Ref<IDXGIDebug1>			DXGIDebug;
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
		D3D12Device() { Create(); }
		~D3D12Device() { Release(); }

		D3D12Features		Features;
		D3D12Capabilities	Capabilities;

		uint32 CommandQueuePriority = 0;

		IDXGIFactory7* GetFactory() { return m_Factory.Get(); }
		IDXGIAdapter4* GetAdapter() { return m_Adapter.Get(); }
		ID3D12Device8* GetDevice()  { return m_Device.Get();  }

		void WaitForGPU();
		void FlushGPU();
		void IdleGPU();

		D3D12Fence* GetFence() { return m_Fence.get(); }

		D3D12Queue* GetGfxQueue()		{ return m_GfxQueue.get(); }
		[[maybe_unused]]
		D3D12Queue* GetComputeQueue()	{ return m_ComputeQueue.get(); }
		[[maybe_unused]]
		D3D12Queue* GetUploadQueue()	{ return m_UploadQueue.get(); }

	private:
		Ref<IDXGIFactory7> m_Factory;
		Ref<IDXGIAdapter4> m_Adapter;
		Ref<ID3D12Device8> m_Device;

		std::unique_ptr<D3D12Fence> m_Fence;

		std::unique_ptr<D3D12Queue> m_GfxQueue;
		std::unique_ptr<D3D12Queue> m_ComputeQueue;
		std::unique_ptr<D3D12Queue> m_UploadQueue;

		std::unique_ptr<D3D12DescriptorHeap> m_SRVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_DSVHeap;
		std::unique_ptr<D3D12DescriptorHeap> m_RTVHeap;

	private:
		void Create();
		void Release();

		void CreateAdapter();
		void CreateDevice();

		void QueryShaderModel() /* Check if desired SM6.6 is supported. */;
		void QueryFeatures();	/* Gather Device features. */

		void CreateQueues();
		void CreateHeaps();


	private:

		DXGI_ADAPTER_DESC3 m_AdapterDesc{};

#if DEBUG_MODE
		D3D12Debug m_DebugDevices;
#endif

	public:

		Buffer*			CreateBuffer(BufferDesc Desc) override final;
		ConstantBuffer* CreateConstantBuffer(void* pData, usize Size) override final;
		Texture*		CreateTexture(TextureDesc Desc) override final;

	private:

	};
} // namespace mf::RHI
