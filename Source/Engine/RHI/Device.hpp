#pragma once

#include <RHI/Types.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/Texture.hpp>
#include <deque>
#include <functional>

namespace lde::RHI
{
	enum class BackendAPI
	{
		eDefault,
		eD3D12,
		eVulkan
	};

	/**
	 * @brief Determines some properties used in device creation
	 * Device can be created with debug flags and validation even if debug mode is off.
	*/
	struct DeviceDesc
	{
		BackendAPI Backend = BackendAPI::eD3D12;
		bool bDebugMode = true;
		bool bEnableValidation = true;
		bool bRaytracing = false;
		bool bMeshShading = false;
	};

	class Device
	{
	public:
		DeviceDesc&		GetDeviceDesc() { return m_DeviceDesc;	}
		DeviceVendor&	GetGpuVendor()	{ return m_GpuVendor;	}
		DeviceType&		GetGpuType()	{ return m_GpuType;		}

		bool IsDebugMode() const { return m_DeviceDesc.bDebugMode; }

		/* ================================= Create resources ================================= */

		virtual BufferHandle	CreateBuffer(BufferDesc Desc) = 0;
		virtual BufferHandle	CreateConstantBuffer(void* pData, usize Size) = 0;
		//virtual TextureHandle	CreateTexture(TextureDesc Desc) = 0;
		//virtual Texture*		CreateTexture(TextureDesc Desc) = 0;

		// TODOs:
		//virtual Buffer* CreateBLAS() = 0;
		//virtual Buffer* CreateTLAS() = 0;

	protected:
		DeviceDesc		m_DeviceDesc{};
		DeviceVendor	m_GpuVendor = DeviceVendor::eUnspecified;
		DeviceType		m_GpuType = DeviceType::eUnspecifed;

	};
} // namespace lde::RHI
