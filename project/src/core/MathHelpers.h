#ifndef MATHHELPERS_H
#define MATHHELPERS_H

#include <cmath>
#include <cfloat>

#include <algorithm>

namespace mau
{
	/* --- CONSOLE COLORS --- */
	inline constexpr char const* RESET  = "\033[0m";
	inline constexpr char const* RED    = "\033[31m";
	inline constexpr char const* GREEN  = "\033[32m";
	inline constexpr char const* YELLOW = "\033[33m";

	/* --- CONSTANTS --- */
	constexpr auto PI = 3.14159265358979323846f;
	constexpr auto PI_DIV_2 = 1.57079632679489661923f;
	constexpr auto PI_DIV_4 = 0.785398163397448309616f;
	constexpr auto PI_2 = 6.283185307179586476925f;
	constexpr auto PI_4 = 12.56637061435917295385f;

	constexpr auto TO_DEGREES = (180.0f / PI);
	constexpr auto TO_RADIANS(PI / 180.0f);

	inline float Square(float a)
	{
		return a * a;
	}

	inline float Lerpf(float a, float b, float factor)
	{
		return ((1 - factor) * a) + (factor * b);
	}

	inline bool AreEqual(float a, float b, float epsilon = FLT_EPSILON)
	{
		return std::abs(a - b) < epsilon;
	}
}

#endif
