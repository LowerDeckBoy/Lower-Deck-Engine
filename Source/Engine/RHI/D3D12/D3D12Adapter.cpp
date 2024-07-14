#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"
#include "D3D12RootSignature.hpp"
//#include "D3D12Memory.hpp"

/*
	Implemets Factory, Adapter, Device, Queues and Heaps creation from D3D12Device.hpp
*/

/* ===========================================  Setting AgilitySDK =========================================== */
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 611;			}
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";	}

namespace lde::RHI
{
	constexpr auto MINIMUM_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;
	constexpr auto DESIRED_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_2;
	
	void D3D12Device::Create()
	{
		// Skip if already initialized
		if (m_Factory.Get())
			return;

		uint32 factoryFlags = 0;
		
#if DEBUG_MODE
		// Enable debug layers
		// Those must be set BEFORE creating Factory and other stuff

		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		DX_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugDevices.D3DDebug)));
		m_DebugDevices.D3DDebug->EnableDebugLayer();
		// Investiage: resizing window while GPU validation is on causes increases in memory usage.
		m_DebugDevices.D3DDebug->SetEnableGPUBasedValidation(FALSE);
#endif

		DX_CALL(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_Factory)));

		CreateAdapter();
		CreateDevice();
		
		QueryShaderModel();
		QueryFeatures();

		m_Fence = std::make_unique<D3D12Fence>(this);

		CreateHeaps();

		CreateFrameResources();
		
	}

	void D3D12Device::Release()
	{
		for (usize i = 0; i < FRAME_COUNT; ++i)
		{
			delete m_FrameResources[i].GraphicsCommandList;
			//delete m_FrameResources[i].ComputeCommandList;
		}
		delete GraphicsQueue;
		//delete ComputeQueue;

		m_RTVHeap.reset();
		m_DSVHeap.reset();
		m_SRVHeap.reset();

		m_Fence.reset();

		SAFE_RELEASE(m_Device);
		SAFE_RELEASE(m_Adapter);
		SAFE_RELEASE(m_Factory);

#if DEBUG_MODE
		// Release debug adapters. They have to be relased at the end, otherwise there will be false-positive RLO detected.
		SAFE_RELEASE(m_DebugDevices.DebugDevice);
		SAFE_RELEASE(m_DebugDevices.D3DDebug);
		m_DebugDevices.DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		SAFE_RELEASE(m_DebugDevices.DXGIDebug);
#endif
	}

	void D3D12Device::CreateAdapter()
	{
		Ref<IDXGIAdapter4> adapter;

		for (uint32 i = 0;
			m_Factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), MINIMUM_FEATURE_LEVEL, __uuidof(ID3D12Device8), nullptr)))
			{
				m_Adapter = adapter;
				SAFE_RELEASE(adapter);
				break;
			}
		}

		LDE_ASSERT(m_Adapter.Get());
		m_Adapter->GetDesc3(&m_AdapterDesc);

	}

	void D3D12Device::CreateDevice()
	{
		// If can't create Device with 12_2 feature level, fallback to 12_0
		if (FAILED(D3D12CreateDevice(m_Adapter.Get(), DESIRED_FEATURE_LEVEL, IID_PPV_ARGS(&m_Device))))
		{
			DX_CALL(D3D12CreateDevice(m_Adapter.Get(), MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&m_Device)));
			SET_D3D12_NAME(m_Device, "D3D12 Logical Device");
		}
		
#if DEBUG_MODE
		// Create Debug interfaces
		DX_CALL(m_Device->QueryInterface(&m_DebugDevices.DebugDevice));
		DX_CALL(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_DebugDevices.DXGIDebug)));
#endif
	}

	void D3D12Device::QueryShaderModel()
	{
		D3D12_FEATURE_DATA_SHADER_MODEL shaderModel{};
		#if defined(NTDDI_WIN10_VB) && (NTDDI_VERSION >= NTDDI_WIN10_VB)
			#define HLSL_SM6_6 1
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_6;
		#elif defined (NTDDI_WIN10_19H1) && (NTDDI_VERSION >= NTDDI_WIN10_19H1)
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_5;
		#elif defined (NTDDI_WIN10_RS5) && (NTDDI_VERSION >= NTDDI_WIN10_RS5)
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_4;
		#elif defined (NTDDI_WIN10_RS4) && (NTDDI_VERSION >= NTDDI_WIN10_RS4)
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_2;
		#elif defined (NTDDI_WIN10_RS3) && (NTDDI_VERSION >= NTDDI_WIN10_RS3)
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_1;
		#else
			#define HLSL_USE_SM6_6 0
			shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_0;
		#endif

		Capabilities.HighestShaderModel = shaderModel.HighestShaderModel;

	}

	void D3D12Device::QueryFeatures()
	{
		/* Gather all desired / expected features */
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,  &Features.Features,  sizeof(Features.Features)) );
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &Features.Features1, sizeof(Features.Features1)));
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &Features.Features3, sizeof(Features.Features3)));
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Features.Features5, sizeof(Features.Features5)));
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &Features.Features6, sizeof(Features.Features6)));
		DX_CALL(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &Features.Features7, sizeof(Features.Features7)));

		Capabilities.RaytracingTier = Features.Features5.RaytracingTier;
		Capabilities.BindingTier	= Features.Features.ResourceBindingTier;
		Capabilities.MeshShaderTier = Features.Features7.MeshShaderTier;
		
	}
	
	void D3D12Device::CreateHeaps()
	{
		// Needs to be bigger as is also meant for generating mip chains.
		m_SRVHeap = std::make_unique<D3D12DescriptorHeap>(this, HeapType::eSRV, 65536, "SRV Descriptor Heap");
		m_DSVHeap = std::make_unique<D3D12DescriptorHeap>(this, HeapType::eDSV, 64,    "DSV Descriptor Heap");
		m_RTVHeap = std::make_unique<D3D12DescriptorHeap>(this, HeapType::eRTV, 64,    "RTV Descriptor Heap");
	}
	
} // namespace lde::RHI
