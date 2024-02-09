#pragma once
#include "GeomertryGenerator/GeometryGenerator.h"
#include "Types.h"

enum class ETextureMapType
{
	None,
	Spherical
};

class IMeshParser
{
public:
	virtual ~IMeshParser() = default;
	virtual bool ParseMesh(const string& Path, OGeometryGenerator::SMeshData& MeshData, ETextureMapType = ETextureMapType::None) = 0;

	template<typename T, typename... Args>
	static unique_ptr<T> CreateParser(Args&&... args)
	{
		return make_unique<T>(std::forward<Args>(args)...);
	}

private:
};

class OCustomParser : public IMeshParser
{
	bool ParseMesh(const string& Path, OGeometryGenerator::SMeshData& MeshData, ETextureMapType Type = ETextureMapType::None) override;
};
