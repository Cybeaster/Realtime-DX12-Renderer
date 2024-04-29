
#include "LightComponent.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/CSM/Csm.h"
#include "Profiler.h"
#include "Window/Window.h"

OLightComponent::OLightComponent()

{
	Name = "LightComponent";
	OnLightChanged.Add([this]() {
		LOG(Render, Log, "Light changed");
		SetLightSourceData();
	});
}

void OLightComponent::Tick(UpdateEventArgs Arg)
{
	PROFILE_SCOPE();
	if (!Utils::MatricesEqual(Owner->GetDefaultInstance()->World, Position)) // make delegate based
	{
		DirectX::XMVECTOR scale, rotation, translation;
		DirectX::XMMatrixDecompose(&scale, &rotation, &translation, DirectX::XMLoadFloat4x4(&Owner->GetDefaultInstance()->World));
		Position = Owner->GetDefaultInstance()->World;
		DirectX::XMFLOAT4 pos{};
		DirectX::XMStoreFloat4(&pos, translation);
		GlobalPosition = translation;

		OnLightChanged.Broadcast();
		MarkDirty();
	}
}

void OLightComponent::Init(ORenderItem* Other)
{
	OSceneComponent::Init(Other);
}

DirectX::XMVECTOR OLightComponent::GetGlobalPosition() const
{
	return GlobalPosition;
}

bool OLightComponent::TryUpdate()
{
	bool res = NumFramesDirty > 0;
	NumFramesDirty = 0;
	return res;
}

void OLightComponent::MarkDirty()
{
	LOG(Render, Log, "Marking light as dirty");
	NumFramesDirty = SRenderConstants::NumFrameResources;
	OnLightChanged.Broadcast();
}

void OLightComponent::UpdateLightData()
{
	PROFILE_SCOPE();
	bIsDirty = true;
}

bool OLightComponent::UseDirty()
{
	if (bIsDirty)
	{
		bIsDirty = false;
		return true;
	}
	return false;
}

void ODirectionalLightComponent::SetShadowMapIndices(array<UINT, 3> Maps)
{
}

HLSL::DirectionalLight& ODirectionalLightComponent::GetDirectionalLight()
{
	return DirectionalLight;
}

ELightType ODirectionalLightComponent::GetLightType() const
{
	return ELightType::Directional;
}

void ODirectionalLightComponent::SetDirectionalLight(const SDirectionalLightPayload& Light)
{
	DirectionalLight.Direction = Light.Direction;
	DirectionalLight.Intensity = Light.Intensity;
	UpdateLightData();
}
int32_t ODirectionalLightComponent::GetLightIndex() const
{
	return DirLightBufferInfo.StartIndex;
}

void ODirectionalLightComponent::UpdateFrameResource(const SFrameResource* FrameResource)
{
	FrameResource->DirectionalLightBuffer->CopyData(DirLightBufferInfo.StartIndex, DirectionalLight);
}

void ODirectionalLightComponent::InitFrameResource(const TUploadBufferData<HLSL::DirectionalLight>& Spot)
{
	DirLightBufferInfo = Spot;
}

void ODirectionalLightComponent::SetPassConstant(SPassConstants& OutConstant)
{
}

void ODirectionalLightComponent::SetCSM(OCSM* InCSM)
{
	CSM = InCSM;
	auto array = MakeShadowMapDataArray();
	for (size_t i = 0; i < 3; i++)
	{
		array[i]->ShadowMapIndex = CSM->GetShadowMap(i)->GetShadowMapIndex();
	}
}

array<HLSL::ShadowMapData*, 3> ODirectionalLightComponent::MakeShadowMapDataArray()
{
	return { &DirectionalLight.ShadowMapDataNear, &DirectionalLight.ShadowMapDataMid, &DirectionalLight.ShadowMapDataFar };
}

OPointLightComponent::OPointLightComponent(uint32_t InIdx)
    : PointLightBufferInfo(InIdx)
{
}

ELightType OPointLightComponent::GetLightType() const
{
	return ELightType::Point;
}

HLSL::PointLight& OPointLightComponent::GetPointLight()
{
	return PointLight;
}

void OPointLightComponent::SetPointLight(const SPointLightPayload& Light)
{
	PointLight.FalloffStart = Light.FallOffStart;
	PointLight.FalloffEnd = Light.FallOffEnd;
	PointLight.Position = Light.Position;
	PointLight.Intensity = Light.Intensity;
	UpdateLightData();
}

void OPointLightComponent::SetShadowMapIndex(UINT Index)
{
	PointLight.ShadowMapIndex = Index;
}

int32_t OPointLightComponent::GetLightIndex() const
{
	return PointLightBufferInfo.StartIndex;
}

void OPointLightComponent::UpdateFrameResource(const SFrameResource* FrameResource)
{
	FrameResource->PointLightBuffer->CopyData(PointLightBufferInfo.StartIndex, PointLight);
}

OSpotLightComponent::OSpotLightComponent(uint32_t InIdx)
    : SpotLightBufferInfo(InIdx)
{
}

void OSpotLightComponent::SetSpotLight(const SSpotLightPayload& Light)
{
	SpotLight.ConeAngle = Light.ConeAngle;
	SpotLight.Direction = Light.Direction;
	SpotLight.FalloffEnd = Light.FallOffEnd;
	SpotLight.FalloffStart = Light.FallOffStart;
	SpotLight.SpotPower = Light.SpotPower;
	SpotLight.Intensity = Light.Strength;
	UpdateLightData();
}

void OSpotLightComponent::SetShadowMapIndex(UINT Index)
{
	SpotLight.ShadowMapIndex = Index;
}

int32_t OSpotLightComponent::GetLightIndex() const
{
	return SpotLightBufferInfo.StartIndex;
}

void OSpotLightComponent::UpdateFrameResource(const SFrameResource* FrameResource)
{
	FrameResource->SpotLightBuffer->CopyData(SpotLightBufferInfo.StartIndex, SpotLight);
}

void OSpotLightComponent::InitFrameResource(const TUploadBufferData<HLSL::SpotLight>& Spot)
{
	SpotLightBufferInfo = Spot;
}

ELightType OSpotLightComponent::GetLightType() const
{
	return ELightType::Spot;
}

void OSpotLightComponent::SetPassConstant(SPassConstants& OutConstant)
{
}

ODirectionalLightComponent::ODirectionalLightComponent(uint32_t InIdx)
    : DirLightBufferInfo(InIdx)
{
}

void ODirectionalLightComponent::SetLightSourceData()
{
	if (CSM == nullptr)
	{
		LOG(Render, Error, "CSM is nullptr");
	}

	using namespace DirectX;
	const XMMATRIX T{
		0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f
	};
	auto zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto camera = OEngine::Get()->GetWindow()->GetCamera();
	auto view = camera->GetView();
	auto frustrumCorners = HLSL::FrustumCornens;
	array<XMVECTOR, 8> corners = {};

	auto shadowTransforms = array{
		&DirectionalLight.ShadowMapDataNear,
		&DirectionalLight.ShadowMapDataMid,
		&DirectionalLight.ShadowMapDataFar
	};

	const auto bounds = OEngine::Get()->GetSceneBounds();
	auto lightDir = XMVector3Normalize(XMLoadFloat3(&DirectionalLight.Direction));
	auto lightPos = lightDir;
	const auto cameraView = camera->GetView();
	const auto cameraProj = camera->GetProj();
	const auto viewProj = cameraView * cameraProj;

	array<float, 3> cascadeSplits;

	float nearClip = camera->GetNearZ();
	float farClip = camera->GetFarZ();

	float clipRange = farClip - nearClip;
	float minZ = nearClip;
	float maxZ = nearClip + clipRange;
	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	for (uint8_t it = 0; it < MAX_CSM_PER_FRAME; it++)
	{
		float cascadeSplitLambda = 0.95f;
		float p = (it + 1) / static_cast<float>(MAX_CSM_PER_FRAME);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[it] = (d - nearClip) / clipRange;
	}

	float lastSplitDist = 0.0f;
	for (int i = 0; i < MAX_CSM_PER_FRAME; i++)
	{
		float splitDist = cascadeSplits[i];
		//project frustrum corners into world space
		auto invViewProj = Inverse(viewProj, false);
		for (int j = 0; j < 8; j++)
		{
			XMVECTOR vec = XMVectorSet(frustrumCorners[j].x, frustrumCorners[j].y, frustrumCorners[j].z, 1.0f); // Set w = 1.0f
			const auto invCorner = XMVector4Transform(vec, invViewProj);
			const auto splat = XMVectorSplatW(invCorner);
			Put(frustrumCorners[j], XMVectorDivide(invCorner, splat));
		}
		for (int j = 0; j < 4; j++)
		{
			auto dist = frustrumCorners[j + 4] - frustrumCorners[j];
			frustrumCorners[j + 4] = frustrumCorners[j] + dist * splitDist;
			frustrumCorners[j] = frustrumCorners[j] + dist * lastSplitDist;
		}

		auto center = zero;
		for (int j = 0; j < 8; j++)
		{
			center = XMVectorAdd(center, Load(frustrumCorners[j]));
		}
		center /= 8.0f;

		float radius = 0.0f;
		for (int j = 0; j < 8; j++)
		{
			float distance = XMVectorGetX(XMVector3Length(Load(frustrumCorners[j]) - center));
			radius = std::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;
		const auto maxExtents = XMVectorSet(radius, radius, radius, 0.0f);
		const auto minExtents = -maxExtents;
		lightDir = XMVector3Normalize(-lightDir);
		const auto eye = center - (lightDir * -XMVectorGetZ(minExtents));
		const auto lightView = MatrixLookAt(eye, center, lightUp);
		const auto orthoMat = MatrixOrthographicOffCenter(GetX(minExtents),
		                                                  GetX(maxExtents),
		                                                  GetY(minExtents),
		                                                  GetY(maxExtents),
		                                                  0,
		                                                  GetZ(maxExtents) - GetZ(minExtents));
		const auto lightOrthoMat = lightView * orthoMat;

		SPassConstants passConstants;
		Put(passConstants.View, lightView);
		Put(passConstants.Proj, orthoMat);
		Put(passConstants.ViewProj, lightOrthoMat);
		Put(passConstants.InvView, Inverse(lightView));

		CSM->GetShadowMap(i)->SetPassConstants(passConstants);
		lastSplitDist = cascadeSplits[i];
	}
}

void OSpotLightComponent::SetLightSourceData()
{
	using namespace DirectX;
	/*auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const auto& bounds = OEngine::Get()->GetSceneBounds();
	// Calculate target point using direction

	auto globalPos = GetGlobalPosition();
	XMVECTOR lightPos = globalPos;
	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&SpotLight.Direction));
	XMVECTOR lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction
	NearZ = SpotLight.FalloffStart;
	FarZ = SpotLight.FalloffEnd;

	// Create the view matrix for the spotlight
	auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
	auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(spotLight.ConeAngle * 2), 1.0f, NearZ, FarZ);
	XMMATRIX S = view * projection * T;
	XMStoreFloat4x4(&View, view);
	XMStoreFloat4x4(&Proj, projection);
	XMStoreFloat4x4(&ShadowTransform, S);*/
}

HLSL::SpotLight& OSpotLightComponent::GetSpotLight()
{
	return SpotLight;
}

void OSpotLightComponent::SetShadowMap(OShadowMap* InShadow)
{
	ShadowMap = InShadow;
}

void OPointLightComponent::InitFrameResource(const TUploadBufferData<HLSL::PointLight>& Spot)
{
	PointLightBufferInfo = Spot;
}
void OPointLightComponent::SetPassConstant(SPassConstants& OutConstant)
{
}

void OPointLightComponent::SetLightSourceData()
{
}
