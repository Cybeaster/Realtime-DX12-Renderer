#pragma once
#include <DirectXColors.h>
#include <Types.h>

class OWaves
{
public:
	OWaves(int32_t M, int32_t N, float dx, float dt, float Speed, float Damping);

	OWaves(const OWaves& rhs) = delete;

	OWaves& operator=(const OWaves& rhs) = delete;

	~OWaves()
	{
	}

	int32_t GetRowCount() const;

	int32_t GetColumnCount() const;

	int32_t GetVertexCount() const;

	int32_t GetTriangleCount() const;

	float GetWidth() const;

	float GetDepth() const;

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& GetPosition(int32_t I) const;

	// Returns the solution normal at the ith grid point.
	const DirectX::XMFLOAT3& GetNormal(int32_t I) const;

	// Returns the unit tangent vector at the ith grid point in the local x-axis direction.
	const DirectX::XMFLOAT3& GetTangentX(int32_t I) const;

	void Update(float dt);

	void Disturb(int32_t I, int32_t J, float Magnitude, float Radius = 50);

private:
	int32_t NumRows;
	int32_t NumCols;

	int32_t VertexCount = 0;
	int32_t TriangleCount = 0;

	float K1 = 0.0f;
	float K2 = 0.0f;
	float K3 = 0.0f;

	float TimeStep = 0.0f;
	float SpatialStep = 0.0f;

	vector<DirectX::XMFLOAT3> PrevSolution;
	vector<DirectX::XMFLOAT3> CurrentSolution;
	vector<DirectX::XMFLOAT3> Normals;
	vector<DirectX::XMFLOAT3> TangentX;
};



