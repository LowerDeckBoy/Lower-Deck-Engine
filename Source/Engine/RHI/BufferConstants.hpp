#pragma once

#include "Core/CoreTypes.hpp"
#include <DirectXMath.h>

namespace lde
{
	struct SceneData
	{
		DirectX::XMVECTOR	CameraPosition;
		DirectX::XMMATRIX	View;
		DirectX::XMMATRIX	Projection;
		DirectX::XMMATRIX	InversedView;
		DirectX::XMMATRIX	InversedProjection;
		uint32				Width;
		uint32				Height;
		float				zNear;
		float				zFar;
	};

	struct PerObject
	{
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixIdentity();

		uint32 VertexBufferIndex = 0;
		uint32 VertexBufferOffset = 0;
		uint32 padding[2]{};

		DirectX::XMFLOAT4 padding2[7];
	};

} // namespace lde
