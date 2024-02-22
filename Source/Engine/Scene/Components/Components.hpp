#pragma once
#include <DirectXMath.h>
#include <string>

namespace lde
{
	using namespace DirectX;

	struct TagComponent
	{
		std::string Name = "Empty";
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& Tag) : Name(Tag) {}
	};
	
	struct TranslationComponent
	{
		XMFLOAT3 Translation;
	};
	
	struct RotationComponent
	{
		XMFLOAT4 Rotation;
	};
	
	struct ScaleComponent
	{
		XMFLOAT3 Scale;
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(DirectX::XMFLOAT3 Position)
			: Translation(Position) { }
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation)
			: Translation(Position), Rotation(Rotation) { }
		TransformComponent(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Rotation, DirectX::XMFLOAT3 Scale)
			: Translation(Position), Rotation(Rotation), Scale(Scale) { }
	
		XMFLOAT3 Translation	= XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 Rotation		= XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 Scale			= XMFLOAT3(1.0f, 1.0f, 1.0f);
		XMMATRIX WorldMatrix	= XMMatrixIdentity();
	
		void Update()
		{
			WorldMatrix = XMMatrixIdentity();
			WorldMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&Scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation))		*XMMatrixTranslationFromVector(XMLoadFloat3(&Translation));
		}
		
		void Reset()
		{
			Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
			Rotation	= XMFLOAT3(0.0f, 0.0f, 0.0f);
			Scale		= XMFLOAT3(1.0f, 1.0f, 1.0f);
			Update();
		}
	
		XMFLOAT3& GetTranslation()
		{
			return Translation;
		}
	};

	struct DirectLightComponent
	{
		XMFLOAT3 Direction;
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
	};
	
	struct PointLightComponent
	{
		XMFLOAT3 Position;
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		float Radius;
	};
	
	struct SpotLightComponent
	{
	
	};

} // namespace lde
