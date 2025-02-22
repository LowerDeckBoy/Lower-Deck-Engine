#pragma once

#include <DirectXMath.h>

namespace lde
{
	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(DirectX::XMFLOAT3 Position)
			: Translation(Position)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation)
			: Translation(Position), Rotation(Rotation)
		{
		}
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation, DirectX::XMFLOAT3 Scale)
			: Translation(Position), Rotation(Rotation), Scale(Scale)
		{
		}

		DirectX::XMFLOAT3 Translation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 Rotation		= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 Scale			= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		DirectX::XMMATRIX WorldMatrix	= DirectX::XMMatrixIdentity();

		void Update()
		{
			WorldMatrix =
				DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&Scale)) *
				DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&Rotation)) *
				DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&Translation));
		}

		void Reset()
		{
			Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			Rotation	= DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
			Scale		= DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

			Update();
		}
	};

} // namespace lde
