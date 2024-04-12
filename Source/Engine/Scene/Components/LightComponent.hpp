#pragma once

/*
	
*/

#include <DirectXMath.h>

namespace lde
{
	using namespace DirectX;

	struct DirectionalLightComponent
	{
		//XMFLOAT4 Position;
		XMFLOAT4 Direction;
		XMFLOAT4 Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		//bool bCastsShadows = false;
	};

	struct PointLightComponent
	{
		XMFLOAT4 Position = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
		XMFLOAT4 Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		float Range = 25.0f;
		XMFLOAT3 padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	struct SpotLightComponent
	{

	};


} // namespace lde
