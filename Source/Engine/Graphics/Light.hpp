#pragma once

#include <Core/Color.hpp>

namespace lde
{
	enum class LightType
	{
		eDirectional,
		ePoint,
		eSpot,
	};

	class Light
	{
	public:
		Light() = default;
		Light(LightType eType);
		~Light() = default;


		bool SetCastShadows(bool bCast);

		bool CastShadows() const { return bCastShadows; }

	private:
		// xyx - RGB; w - intensity (0.0 to 1.0)
		Color m_Color;
		float m_Radius = 1.0f;
		bool bCastShadows = false;

	};
}