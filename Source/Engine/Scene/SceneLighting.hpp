#pragma once

/*
	Scene/SceneLighting.hpp
	Gathers informations about quantity and types of light components
	for given Scene.
	Creates and updates Constant Buffer.
*/

#include "Components/LightComponent.hpp"

#include "RHI/D3D12/D3D12Buffer.hpp"
#include <Core/CoreTypes.hpp>

namespace lde
{
#define MAX_DIR_LIGHTS		4u
#define MAX_POINT_LIGHTS	64u
#define MAX_SPOT_LIGHTS		32u

	enum class LightType
	{
		eDirectional,
		ePoint,
		eSpot,
		eArea,
		COUNT
	};

	enum class LightMobility
	{
		eStationary,
		eDynamic
	};

	enum class LightCullMode
	{
		eCullable,
		eNonCullable
	};

	class SceneLighting
	{
	public:

		void Create();
		void Update();

		void Release();

		void AddDirLight();
		void AddPointLight();
		void AddSpotLight();

	private:
		//std::unique_ptr<RHI::D3D12ConstantBuffer> m_ConstBuffer;

		uint32 m_DirLightsCount		= 0u;
		uint32 m_PointLightsCount	= 0u;
		uint32 m_SpotLightsCount	= 0u;

		RHI::D3D12ConstantBuffer* m_DirLightBuffer;

	};
}
