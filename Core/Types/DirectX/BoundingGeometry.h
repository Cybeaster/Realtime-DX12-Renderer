#pragma once
#include "HLSL/HlslTypes.h"

#include <memory>

enum class EBoundingType
{
	None,
	Box,
	OrientedBox,
	Sphere,
	Frustum
};

inline wstring ToString(EBoundingType Type)
{
	switch (Type)
	{
	case EBoundingType::None:
		return L"None";
	case EBoundingType::Box:
		return L"Box";
	case EBoundingType::OrientedBox:
		return L"OrientedBox";
	case EBoundingType::Sphere:
		return L"Sphere";
	case EBoundingType::Frustum:
		return L"Frustum";
	}
	return L"None";
}

class IBoundingGeometry
{
public:
	virtual ~IBoundingGeometry() = default;
	virtual std::unique_ptr<IBoundingGeometry> Clone() = 0;
	virtual EBoundingType GetType() const = 0;
	virtual DirectX::ContainmentType Contains(const DirectX::XMMATRIX& View, DirectX::BoundingBox) = 0;
};

template<typename GeometryType>
class TBoundingGeometry : public IBoundingGeometry
{
public:
	using BoundingType = GeometryType;
	virtual ~TBoundingGeometry() = default;
	TBoundingGeometry() = default;
	TBoundingGeometry(const TBoundingGeometry& Other)
	    : Geometry(Other.Geometry) {}

	TBoundingGeometry(const GeometryType& Other)
	    : Geometry(Other) {}

	TBoundingGeometry& operator=(const TBoundingGeometry& Other)
	{
		Geometry = Other.Geometry;
		return *this;
	}

	DirectX::ContainmentType Contains(const TBoundingGeometry& other) const
	{
		return Geometry.Contains(other.Geometry);
	}
	DirectX::ContainmentType Contains(const DirectX::BoundingFrustum& frustum) const
	{
		return Geometry.Contains(frustum);
	}
	DirectX::ContainmentType Contains(const DirectX::BoundingBox& box) const
	{
		return Geometry.Contains(box);
	}
	DirectX::ContainmentType Contains(const DirectX::BoundingOrientedBox& box) const
	{
		return Geometry.Contains(box);
	}
	DirectX::ContainmentType Contains(const DirectX::BoundingSphere& sphere) const
	{
		return Geometry.Contains(sphere);
	}

	DirectX::ContainmentType Contains(const DirectX::XMMATRIX& ViewToLocal, DirectX::BoundingBox Box) override
	{
		GeometryType otherGeometry;
		Transform(otherGeometry, ViewToLocal);
		return otherGeometry.Contains(Box);
	}

	virtual void Transform(GeometryType& Out, const DirectX::XMMATRIX& Matrix)
	{
		Geometry.Transform(Out, Matrix);
	}

	EBoundingType GetType() const override
	{
		static_assert(
		    std::is_same_v<GeometryType, DirectX::BoundingBox> || std::is_same_v<GeometryType, DirectX::BoundingOrientedBox> || std::is_same_v<GeometryType, DirectX::BoundingSphere> || std::is_same_v<GeometryType, DirectX::BoundingFrustum>,
		    "Unsupported geometry type");

		if constexpr (std::is_same_v<GeometryType, DirectX::BoundingBox>)
		{
			return EBoundingType::Box;
		}
		else if constexpr (std::is_same_v<GeometryType, DirectX::BoundingOrientedBox>)
		{
			return EBoundingType::OrientedBox;
		}
		else if constexpr (std::is_same_v<GeometryType, DirectX::BoundingSphere>)
		{
			return EBoundingType::Sphere;
		}
		else if constexpr (std::is_same_v<GeometryType, DirectX::BoundingFrustum>)
		{
			return EBoundingType::Frustum;
		}
		return EBoundingType::None;
	}

	virtual void Transform(TBoundingGeometry& Out, const DirectX::XMMATRIX& Matrix)
	{
		Geometry.Transform(Out.Geometry, Matrix);
	}

	void ConstructFromGeometry(const GeometryType& geometry)
	{
		Geometry = geometry;
	}

	[[nodiscard]] virtual std::unique_ptr<IBoundingGeometry> Clone() override
	{
		return std::make_unique<TBoundingGeometry>(*this);
	}

protected:
	BoundingType Geometry;
};

class OBoundingBox : public TBoundingGeometry<DirectX::BoundingBox>
{
public:
	OBoundingBox() = default;
	OBoundingBox(const DirectX::BoundingBox& Box)
	    : TBoundingGeometry(Box) {}
};

class OBoundingOrientedBox : public TBoundingGeometry<DirectX::BoundingOrientedBox>
{
public:
	OBoundingOrientedBox() = default;
	OBoundingOrientedBox(const DirectX::BoundingOrientedBox& Box)
	    : TBoundingGeometry(Box) {}
};

class OBoundingSphere : public TBoundingGeometry<DirectX::BoundingSphere>
{
public:
	OBoundingSphere() = default;

	OBoundingSphere(const DirectX::BoundingSphere& Sphere)
	    : TBoundingGeometry(Sphere) {}
};

class OBoundingFrustum : public TBoundingGeometry<DirectX::BoundingFrustum>
{
public:
	OBoundingFrustum(const DirectX::BoundingFrustum& Frustum)
	    : TBoundingGeometry(Frustum) {}
	OBoundingFrustum() = default;
};
