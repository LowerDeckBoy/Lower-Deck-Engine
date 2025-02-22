#pragma once
#include <DirectXMath.h>
#include <string>

namespace lde
{
	using namespace DirectX;

	struct TranslationComponent
	{
		XMFLOAT3 Translation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};
	
	struct RotationComponent
	{
		XMFLOAT4 Rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	};
	
	struct ScaleComponent
	{
		XMFLOAT3 Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	};

} // namespace lde
