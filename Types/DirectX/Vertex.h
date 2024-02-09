#pragma once
#include <DirectXMath.h>

struct SVertex
{
	SVertex() = default;
	SVertex(float X, float Y, float Z, float Nx, float Ny, float Nz, float U, float V)
	    : Position(X, Y, Z)
	    , Normal(Nx, Ny, Nz)
	    , TexC(U, V)
	{
	}

	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
};