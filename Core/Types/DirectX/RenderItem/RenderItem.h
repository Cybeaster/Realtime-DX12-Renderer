#pragma once
#include "../../Materials/Material.h"
#include "Color.h"
#include "Components/LightComponent/LightComponent.h"
#include "Components/RenderItemComponentBase.h"
#include "DirectX/MeshGeometry.h"
#include "Logger.h"
#include "Transform.h"

struct SFrameResource;

struct SRenderItemGeometry
{
};

struct SInstanceData
{
	DECLARE_DELEGATE(SPositionChanged, STransform);
	SInstanceData() = default;
	SInstanceData(const SInstanceData& In)
	{
		HlslData = In.HlslData;
		Lifetime = In.Lifetime;
	}

	SInstanceData& operator=(const SInstanceData& In)
	{
		HlslData = In.HlslData;
		Lifetime = In.Lifetime;
		return *this;
	}

	HLSL::InstanceData HlslData;
	std::optional<float> Lifetime;
	SPositionChanged PositionChanged;
};

/**
 * @brief Object placeable in the world, having geomentry, material and variable number of instances
 */
struct ORenderItem
{
	ORenderItem() = default;
	ORenderItem(ORenderItem&&) = default;
	ORenderItem(const ORenderItem&) = default;

	void Update(const UpdateEventArgs& Arg);
	bool IsValid() const;
	bool IsValidChecked() const;

	template<typename T, typename... Args>
	T* AddComponent(Args&&... Arg);
	void AddInstance(const SInstanceData& Instance);
	SRenderLayer RenderLayer = "NONE";

	string Name;
	bool bTraceable = true;
	bool bFrustumCoolingEnabled = true;

	weak_ptr<SMaterial> DefaultMaterial;
	weak_ptr<SMeshGeometry> Geometry;
	weak_ptr<SSubmeshGeometry> ChosenSubmesh;
	DirectX::BoundingBox Bounds;
	vector<SInstanceData> Instances;

	/*UI*/
	bool bIsDisplayable = true;
	SInstanceData* GetDefaultInstance();
	const vector<unique_ptr<OComponentBase>>& GetComponents() const;

private:
	vector<unique_ptr<OComponentBase>> Components;
};

struct SCulledRenderItem
{
	weak_ptr<ORenderItem> Item;
	UINT StartInstanceLocation = 0;
	UINT VisibleInstanceCount = 0;
};

struct SCulledInstancesInfo
{
	unordered_map<weak_ptr<ORenderItem>, SCulledRenderItem> Items = {};
	TUUID BufferId;
	uint32_t InstanceCount = 0;
};

template<typename T, typename... Args>
T* ORenderItem::AddComponent(Args&&... Arg)
{
	auto component = make_unique<T>(std::forward<Args>(Arg)...);
	auto res = component.get();
	Components.push_back(move(component));
	Components.back()->Init(this);
	return res;
}

struct SRenderItemParams
{
	SRenderItemParams(const weak_ptr<SMaterial>& InMaterial)
	    : Material(InMaterial) {}
	SRenderItemParams() = default;

	SRenderItemParams(const weak_ptr<SMaterial>& InMaterial, size_t Instances)
	    : Material(InMaterial), NumberOfInstances(Instances) {}

	std::optional<SRenderLayer> OverrideLayer;

	string Submesh;
	weak_ptr<SMaterial> Material;
	size_t NumberOfInstances = 1;
	bool bFrustumCoolingEnabled = false;
	bool Pickable = false;
	void SetPosition(DirectX::XMFLOAT3 P);
	void SetScale(DirectX::XMFLOAT3 S);

	std::optional<DirectX::XMFLOAT3> Position;
	std::optional<DirectX::XMFLOAT3> Scale;
	//Quaternion
	std::optional<DirectX::XMFLOAT4> Rotation;
	bool Displayable = true;
	std::optional<SColor> OverrideColor = SColor::White;
	std::optional<float> Lifetime;
};