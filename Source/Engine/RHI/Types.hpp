#pragma once

/*
	RHI/Types.hpp
	
*/

#include <Core/CoreTypes.hpp>
#include <Core/String.hpp>

namespace mf::RHI
{
#define GPU_VENDOR_NVIDIA	0x10DE
#define GPU_VENDOR_AMD		0x1002
#define GPU_VENDOR_INTEL	0x8086

	enum class DeviceVendor
	{
		eUnspecified	= 0,
		eNvdia			= 1 << 0,
		eAMD			= 1 << 1,
		eIntel			= 1 << 2,
	};

	enum class DeviceType
	{
		eUnspecifed = 0,
		eDiscrete	= 1 << 0,
		eIntegrated	= 1 << 1,
		eVirtual	= 1 << 2, 
	};

	enum class ShaderStage
	{
		eVertex,
		ePixel,
		eCompute,
		eAmplification,
		eMesh,
		eRaytracing, /* Fall-through to the same target */
		eUnspecified,
		COUNT
	};

	enum class PipelineType
	{
		eGraphics,
		eCompute
	};

	enum class CommandType
	{
		eGraphics,
		eCompute,
		eUpload,
		eBundle
	};

	enum class CullMode
	{
		eNone,
		eBack,
		eFront
	};

	enum class Format
	{
		eRGBA8_UNORM,
		eRGBA16_UNORM,
		eRGBA32_UNORM,

		eRGBA8_FLOAT,
		eRGBA16_FLOAT,
		eRGBA32_FLOAT,
		// Depth
		eD32_FLOAT,
		eD24S8_FLOAT,
		COUNT
	};

	enum class ResourceState
	{
		eGeneralUsage,
		eRenderTarget,
		ePresent,
		eCopySrc,
		eCopyDst
	};

	std::wstring ShaderEnumToType(ShaderStage eStage);

}
