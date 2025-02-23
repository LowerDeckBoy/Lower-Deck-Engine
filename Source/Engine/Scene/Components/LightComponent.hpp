#pragma once

#include <DirectXMath.h>

namespace lde
{
	struct DirectionalLightComponent
	{
		DirectionalLightComponent() = default;
		DirectionalLightComponent(DirectX::XMFLOAT3 Direction) 
			: Direction(Direction) 
		{ }

		DirectX::XMFLOAT3	Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
		// Range [0.0, 1.0] - [not visible; fully visible]
		float				Visibility = 1.0f;
		DirectX::XMFLOAT4	Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	};

	struct PointLightComponent
	{
		PointLightComponent() = default;
		PointLightComponent(DirectX::XMFLOAT3 Position)
			: Position(Position)
		{ }

		DirectX::XMFLOAT3	Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
		// Range [0.0, 1.0] - [not visible; fully visible]
		float				Visibility = 1.0f;
		DirectX::XMFLOAT4	Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		float				Range = 25.0f;
		DirectX::XMFLOAT3	padding = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

} // namespace lde
