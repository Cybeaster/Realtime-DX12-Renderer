#pragma once
#include "DXHelper.h"

#include <random>

namespace Utils::Math
{
template<typename T>
T Clamp(const T& x, const T& Low, const T& High)
{
	return x < Low ? Low : (x > High ? High : x);
}

//clang-format off
inline DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
	    1.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 1.0f,
	    0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f,
	    1.0f);

	return I;
}
//clang-format on

template<typename Type>
inline auto Lerp(float T, const Type& A, const Type& B)
{
	return (1.0f - T) * A + T * B; // The interpolated vector
}

template<typename Type>
inline auto IsNormalized(const Type& A, float Epsilon = 1e-6f)
{
	return abs(1.0f - Length(A)) < Epsilon; // Return true if the vector is normalized
}

template<typename T>
auto Squared(T Value)
{
	return Value * Value;
}

template<typename Type>
inline auto DegreesToRaians(auto Degrees)
{
	return Degrees * (DirectX::XM_PI / 180.0f);
}

template<typename T = double>
inline auto Random()
{
	static std::uniform_real_distribution<T> Distribution(0, 1); // Random double distribution
	static std::mt19937 Generator; // Random number generator
	return Distribution(Generator); // Return a random double between 0 and 1
}

template<typename T = double>
inline auto Random(T Min, T Max)
{
	return Min + (Max - Min) * Random<T>(); // Return a random double between Min and Max
}

inline int32_t Random(int32_t Min, int32_t Max)
{
	return Random<double>(Min, Max + 1);
}
}
