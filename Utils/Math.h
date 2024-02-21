#pragma once
#include "DirectX/DXHelper.h"

#include <DirectXMath.h>
#include <corecrt_math.h>

#include <random>

namespace Utils::Math
{
constexpr float Infinity = FLT_MAX;

template<typename T>
T Clamp(const T& x, const T& Low, const T& High)
{
	return x < Low ? Low : (x > High ? High : x);
}

//clang-format off
inline DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
	    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

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

inline DirectX::XMMATRIX InverseTranspose(DirectX::CXMMATRIX Matrix)
{
	DirectX::XMMATRIX A = Matrix;
	A.r[3] = DirectX::XMVectorSet(0.f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR Determinant = DirectX::XMMatrixDeterminant(A);
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&Determinant, A));
}

inline DirectX::XMVECTOR SphericalToCartesian(float Radius, float Theta, float Phi)
{
	return DirectX::XMVectorSet(
	    Radius * cosf(Theta) * sinf(Phi),
	    Radius * cosf(Phi),
	    Radius * sinf(Theta) * sinf(Phi),
	    1.0f);
}
} // namespace Utils::Math

inline DirectX::XMFLOAT4X4 Scale(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 ScaleFactor)
{
	DirectX::XMStoreFloat4x4(&OutOther, DirectX::XMMatrixScaling(ScaleFactor.x, ScaleFactor.y, ScaleFactor.z));
	return OutOther;
}

inline DirectX::XMFLOAT4X4 Scale(const DirectX::XMFLOAT4X4& Other, DirectX::XMFLOAT3 ScaleFactor)
{
	auto other = Other;
	DirectX::XMStoreFloat4x4(&other, DirectX::XMMatrixScaling(ScaleFactor.x, ScaleFactor.y, ScaleFactor.z));
	return other;
}

inline DirectX::XMFLOAT4X4 Translate(DirectX::XMFLOAT4X4& OutOther, DirectX::XMFLOAT3 TranslationFactor)
{
	DirectX::XMStoreFloat4x4(&OutOther, DirectX::XMMatrixTranslation(TranslationFactor.x, TranslationFactor.y, TranslationFactor.z));
	return OutOther;
}

inline DirectX::XMFLOAT4X4 Translate(const DirectX::XMFLOAT4X4& Other, DirectX::XMFLOAT3 TranslationFactor)
{
	auto other = Other;
	XMStoreFloat4x4(&other, DirectX::XMMatrixTranslation(TranslationFactor.x, TranslationFactor.y, TranslationFactor.z));
	return other;
}
