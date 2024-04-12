#pragma once

/*
	Engine/Scene/SceneLighting.hpp
	Gathers informations about quantity and types of light components
	for given Scene.
	Creates and updates Constant Buffer.
*/

#include "Components/LightComponent.hpp"

#include "RHI/D3D12/D3D12Buffer.hpp"
#include <Core/CoreTypes.hpp>
//#include "World.hpp"
#include "Entity.hpp"

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
		eNoncullable
	};

	class SceneLighting : public Entity
	{
	public:
		SceneLighting(RHI::D3D12RHI* pGfx, World* pWorld);
		~SceneLighting();

		void Create();
		void Update();

		void Release();

		void AddDirectionalLight();
		void AddPointLight();
		void AddSpotLight();

		RHI::ConstantBuffer* GetDirectionalLightBuffer() { return m_DirLightBuffer; }

	private:
		RHI::D3D12RHI* m_Gfx = nullptr;
		// Parent world;
		World* m_World = nullptr;
		//std::unique_ptr<RHI::D3D12ConstantBuffer> m_ConstBuffer;
		 
		uint32 m_DirLightsCount		= 0u;
		uint32 m_PointLightsCount	= 0u;
		uint32 m_SpotLightsCount	= 0u;

		RHI::ConstantBuffer* m_DirLightBuffer = nullptr;
		DirectionalLightComponent m_DirLightData{};

	};
}
