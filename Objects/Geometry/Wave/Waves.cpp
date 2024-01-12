//
// Created by Cybea on 27/12/2023.
//

#include "Waves.h"
#include "ppl.h"

#include "Exception.h"

#include <pplwin.h>

OWaves::OWaves(int32_t M, int32_t N, float dx, float dt, float Speed, float Damping)
{
	NumRows = M;
	NumCols = N;

	VertexCount = M * N;
	TriangleCount = (M - 1) * (N - 1) * 2;

	TimeStep = dt;
	SpatialStep = dx;

	float d = Damping * dt + 2.0f;
	float e = (Speed * Speed) * (dt * dt) / (dx * dx);

	K1 = (Damping * dt - 2.0f) / d;
	K2 = (4.0f - 8.0f * e) / d;
	K3 = (2.0f * e) / d;

	PrevSolution.resize(VertexCount);
	CurrentSolution.resize(VertexCount);
	Normals.resize(VertexCount);
	TangentX.resize(VertexCount);

	//generate grid vertices in system memory

	const float halfWidth = (N - 1) * dx * 0.5f;
	const float halfDepth = (M - 1) * dx * 0.5f;
	for (int i = 0; i < M; i++)
	{
		float z = halfDepth - i * dx;
		for (int j = 0; j < N; j++)
		{
			float x = -halfWidth + j * dx;

			PrevSolution[i * N + j] = { x, 0.0f, z };
			CurrentSolution[i * N + j] = { x, 0.0f, z };
			Normals[i * N + j] = { 0.0f, 1.0f, 0.0f };
			TangentX[i * N + j] = { 1.0f, 0.0f, 0.0f };
		}
	}
}

int32_t OWaves::GetRowCount() const
{
	return NumRows;
}

int32_t OWaves::GetColumnCount() const
{
	return NumCols;
}

int32_t OWaves::GetVertexCount() const
{
	return VertexCount;
}

int32_t OWaves::GetTriangleCount() const
{
	return TriangleCount;;
}

float OWaves::GetWidth() const
{
	return NumCols * SpatialStep;
}

float OWaves::GetDepth() const
{
	return NumRows * SpatialStep;
}

const DirectX::XMFLOAT3& OWaves::GetPosition(int32_t I) const
{
	return CurrentSolution[I];
}

const DirectX::XMFLOAT3& OWaves::GetNormal(int32_t I) const
{
	return Normals[I];
}

const DirectX::XMFLOAT3& OWaves::GetTangentX(int32_t I) const
{
	return TangentX[I];
}

void OWaves::Update(float dt)
{
	static float t = 0;

	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= TimeStep)
	{
		// Only update interior points; we use zero boundary conditions.
		concurrency::parallel_for(1,
		                          NumRows - 1,
		                          [this](int32_t I)
		                          {
			                          for (int32_t j = 1; j < NumCols - 1; ++j)
			                          {
				                          // After this update we will be discarding the old previous
				                          // buffer, so overwrite that buffer with the new update.
				                          // Note how we can do this inplace (read/write to same element)
				                          // because we won't need prev_ij again and the assignment happens last.

				                          // Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				                          // Moreover, our +z axis goes "down"; this is just to
				                          // keep consistent with our row indices going down.

				                          PrevSolution[I * NumCols + j].y = K1 * PrevSolution[I * NumCols + j].y +
				                                                            K2 * CurrentSolution[I * NumCols + j].y +
				                                                            K3 * (CurrentSolution[(I + 1) * NumCols + j].y +
				                                                                  CurrentSolution[(I - 1) * NumCols + j].y +
				                                                                  CurrentSolution[I * NumCols + j + 1].y +
				                                                                  CurrentSolution[I * NumCols + j - 1].y); // The new value depends on the old value and its four neighbors
			                          }
		                          });

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.

		std::swap(PrevSolution, CurrentSolution);
		t = 0.0;

		//
		// Compute normals using finite difference scheme.
		//
		concurrency::parallel_for(1,
		                          NumRows - 1,
		                          [this](int32_t I)
		                          {
			                          for (int32_t j = 1; j < NumCols - 1; ++j)
			                          {
				                          float l = CurrentSolution[I * NumCols + j - 1].y;
				                          float r = CurrentSolution[I * NumCols + j + 1].y;
				                          float t = CurrentSolution[(I - 1) * NumCols + j].y;
				                          float b = CurrentSolution[(I + 1) * NumCols + j].y;

				                          Normals[I * NumCols + j].x = -r + l;
				                          Normals[I * NumCols + j].y = 2.0f * SpatialStep;
				                          Normals[I * NumCols + j].z = b - t;

				                          DirectX::XMVECTOR n = DirectX::XMVector3Normalize(XMLoadFloat3(&Normals[I * NumCols + j]));
				                          XMStoreFloat3(&Normals[I * NumCols + j], n);

				                          TangentX[I * NumCols + j] = { 2.0f * SpatialStep, r - l, 0.0f };
				                          DirectX::XMVECTOR T = DirectX::XMVector3Normalize(XMLoadFloat3(&TangentX[I * NumCols + j]));
				                          XMStoreFloat3(&TangentX[I * NumCols + j], T);
			                          }
		                          });
	}
}

void OWaves::Disturb(int32_t I, int32_t J, float Magnitude, float Radius)
{
	CHECK(I > 1 && I < NumRows - 2, "I is out of bounds");
	CHECK(J > 1 && J < NumCols - 2, "J is out of bounds");
	CurrentSolution[I * NumCols + J].y += Magnitude;

	float HalfMag = Magnitude;
	for (int i = 1; i < Radius; ++i)
	{
		HalfMag *= 0.7;
		const auto right = I * NumCols + J + i;
		const auto left = I * NumCols + J - i;
		const auto up = (I + i) * NumCols + J;
		const auto down = (I - i) * NumCols + J;

		if (right > 0 && right < CurrentSolution.size())
			CurrentSolution[right].y += HalfMag;

		if (left > 0 && left < CurrentSolution.size())
			CurrentSolution[left].y += HalfMag;

		if (up > 0 && up < CurrentSolution.size())
			CurrentSolution[up].y += HalfMag;

		if (down > 0 && down < CurrentSolution.size())
			CurrentSolution[down].y += HalfMag;
	}
}
