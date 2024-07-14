#pragma once

/*

*/

#include <cmath>

namespace lde
{
#define ALIGN(Value, Alignment) (Value + (Alignment - 1)) & ~(Alignment - 1)

	constexpr const float PI		= 3.1415926535f;
	constexpr const float TwoPI		= 2.0f * PI;
	//constexpr const float Epsilon	= 0.000001f;
	constexpr const float Rad2Deg	= 57.29578f;
	constexpr const float Deg2Rad	= 0.01745329251f;

	static constexpr uint32 Align(uint32 Value, uint32 Alignment)
	{
		return static_cast<uint32>((Value + (Alignment - 1)) & ~(Alignment - 1));
	}

	static constexpr uint64 Align(uint64 Value, uint64 Alignment)
	{
		return static_cast<uint64>((Value + (Alignment - 1)) & ~(Alignment - 1));
	}

	static constexpr uint32 Clamp(uint32 Value, uint32 Min, uint32 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	static constexpr uint64 Clamp(uint64 Value, uint64 Min, uint64 Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	static constexpr float Clamp(float Value, float Min, float Max)
	{
		return (Value < Min) ? Min : (Value > Max) ? Max : Value;
	}

	// TODO:
	//static constexpr Lerp();
}
