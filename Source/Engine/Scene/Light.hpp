#pragma once

#include <DirectXMath.h>

namespace lde
{
	enum class LightType
	{
		eDirectional,
		ePoint,
		eSpot,
		eArea
	};

	struct LightParameters
	{
		DirectX::XMFLOAT4 Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		// xyz - RGB, w - Intensity (0.0 to 1.0)
		DirectX::XMFLOAT4 Ambient  = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		// xyz - RGB, w - Intensity (0.0 to 1.0)
		DirectX::XMFLOAT4 Diffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		float Radius;
		
	};
	
	class Light
	{
	public:
		Light();
		Light(LightType eType, LightParameters Parameters);
		~Light();

		void Create();
		void Update();
		void Release();

		void SetPosition(DirectX::XMFLOAT4 Position);
		void SetAmbient(DirectX::XMFLOAT4 Color);
		void SetDiffuse(DirectX::XMFLOAT4 Color);
		void SetRadius(float Radius);
		void SetIntensity(float Intensity);

	private:
		LightType m_Type{};
		LightParameters m_Parameters{};

	};
}