#pragma once

#include <Core/CoreTypes.hpp>
#include <DirectXMath.h>

namespace lde::RHI
{
	struct SceneData
	{
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Projection;
		DirectX::XMMATRIX InversedView;
		DirectX::XMMATRIX InversedProjection;
		uint32 Width;
		uint32 Height;
		float zNear;
		float zFar;
	};

	struct PerObject
	{
		DirectX::XMMATRIX World;
		// Vertex Buffer Pointer
		uint32 VertexBufferIndex;
		// Vertex Buffer Offset
		uint32 VertexBufferOffset;
	};

} // namespace lde::RHI
