#pragma once
#include "../../Materials/Material.h"
#include "Color.h"
#include "Components/LightComponent/LightComponent.h"
#include "Components/RenderItemComponentBase.h"
#include "DirectX/MeshGeometry.h"
#include "Logger.h"

struct SFrameResource;

struct SRenderItemGeometry
{
};

/**
 * @brief Object placeable in the world, having geomentry, material and variable number of instances
 */
struct ORenderItem
{
	DECLARE_DELEGATE(SPositionChanged, void);

	ORenderItem() = default;
	ORenderItem(ORenderItem&&) = default;
	ORenderItem(const ORenderItem&) = default;

	void Update(const UpdateEventArgs& Arg);
	bool IsValid() const;
	bool IsValidChecked() const;

	template<typename T, typename... Args>
	T* AddComponent(Args&&... Arg);

	SRenderLayer RenderLayer = "NONE";

	string Name;
	bool bTraceable = true;
	bool bFrustrumCoolingEnabled = true;

	SMaterial* DefaultMaterial = nullptr;
	SMeshGeometry* Geometry = nullptr;
	SSubmeshGeometry* ChosenSubmesh = nullptr;
	DirectX::BoundingBox Bounds;
	vector<HLSL::InstanceData> Instances;

	std::optional<float> Lifetime;

	/*UI*/
	bool bIsDisplayable = true;
	HLSL::InstanceData* GetDefaultInstance();
	const vector<unique_ptr<OComponentBase>>& GetComponents() const;
	void OnMaterialChanged();

private:
	vector<unique_ptr<OComponentBase>> Components;
};

struct SCulledRenderItem
{
	ORenderItem* Item = nullptr;
	UINT StartInstanceLocation = 0;
	UINT VisibleInstanceCount = 0;
};

struct SCulledInstancesInfo
{
	unordered_map<ORenderItem*, SCulledRenderItem> Items = {};
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
	SRenderItemParams(SMaterial* Material)
	    : MaterialParams(Material) {}
	SRenderItemParams() = default;

	SRenderItemParams(SMaterial* Material, size_t Instances)
	    : MaterialParams(Material), NumberOfInstances(Instances) {}

	std::optional<SRenderLayer> OverrideLayer;

	string Submesh;
	SMaterialParams MaterialParams;
	size_t NumberOfInstances = 1;
	bool bFrustrumCoolingEnabled = false;
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