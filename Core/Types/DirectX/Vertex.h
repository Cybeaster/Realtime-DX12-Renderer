#pragma once
#include <DirectXMath.h>

struct SVertex
{
	SVertex() = default;
	SVertex(float X, float Y, float Z, float Nx, float Ny, float Nz, float U, float V, float Tx, float Ty, float Tz)
	    : Position(X, Y, Z)
	    , Normal(Nx, Ny, Nz)
	    , TexC(U, V)
	    , TangentU(Tx, Ty, Tz)
	{
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU = { 0.0f, 0.0f, 0.0f };
};