#pragma once

#include <DirectXMath.h>

namespace lde
{
	// Alpha channel default to 1.0 for every color.
	static struct Colors
	{
		DirectX::XMFLOAT4 White = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		DirectX::XMFLOAT4 Black = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT4 Red	= DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT4 Green = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		DirectX::XMFLOAT4 Blue	= DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	};

	// Packed as 4 floats; 3 for RGB, 4th for alpha channel.
	class Color
	{
	public:
		Color() = default;
		Color(float R, float G, float B)
		{
			m_Color = DirectX::XMFLOAT4(R, G, B, 1.0f);
		}
		Color(float R, float G, float B, float A)
		{
			m_Color = DirectX::XMFLOAT4(R, G, B, A);
		}
		Color(DirectX::XMFLOAT4 Color)
		{
			m_Color = Color;
		}
		~Color() = default;

		void SetColor(float R, float G, float B)
		{
			m_Color = DirectX::XMFLOAT4(R, G, B, 1.0f);
		}
		void SetColor(DirectX::XMFLOAT4 Color)
		{
			m_Color = Color;
		}


		DirectX::XMFLOAT4 GetColor() const
		{
			return m_Color;
		}

		float R() const { return m_Color.x; }
		float G() const { return m_Color.y; }
		float B() const { return m_Color.z; }
		float A() const { return m_Color.w; }

	private:
		DirectX::XMFLOAT4 m_Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	};
} // namespace lde
