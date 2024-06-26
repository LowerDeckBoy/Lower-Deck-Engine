#pragma once

/*
	
*/

#include <DirectXMath.h>
#include "RHI/D3D12/D3D12Texture.hpp"

namespace lde
{
	using namespace DirectX;

	// TODO:
	// https://github.com/KatanaMajesty/WarpEngine/blob/master/src/World/Components/LightComponent.h
	struct DirectionalLightShadowMap
	{
		XMMATRIX View;
		XMMATRIX Projection;
		XMMATRIX InvView;
		XMMATRIX InvProjection;
		RHI::D3D12RenderTexture Texture;
		void Render(XMFLOAT3 Position);
	};

	struct DirectionalLightComponent
	{
		DirectionalLightComponent() = default;
		DirectionalLightComponent(XMFLOAT3 Direction) { this->Direction = Direction; }

		XMFLOAT3	Direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
		// Range [0.0, 1.0]
		// 0.0 - not visible,
		// 1.0 - fully visible
		float		Visibility = 1.0f;
		XMFLOAT4	Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4	Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		bool		bCastShadows = false;
		XMFLOAT3	padding{};
		
		//DirectionalLightShadowMap ShadowMap;

	};

	struct PointLightComponent
	{
		PointLightComponent() = default;
		PointLightComponent(XMFLOAT3 Position) { this->Position = Position; }

		XMFLOAT3	Position = XMFLOAT3(0.0f, 1.0f, 0.0f); 
		// Range [0.0, 1.0]
		// 0.0 - not visible,
		// 1.0 - fully visible
		float		Visibility = 1.0f;
		XMFLOAT4	Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		float		Range = 25.0f;
		XMFLOAT3	padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};

	// TODO:
	struct SpotLightComponent
	{

	};

	

	// TODO:
	struct PointLightShadowMap
	{
		XMMATRIX Views[6];
		XMMATRIX Projection;
	};


} // namespace lde
