
#include "LightComponent.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/CSM/Csm.h"
#include "MathUtils.h"
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
	auto instance = Owner->GetDefaultInstance();
	if (instance->Position != Position || DirectX::XMQuaternionEqual(Load(instance->Rotation), Load(Rotation)) || instance->Scale != Scale) // make delegate based
	{
		WorldMatrix = Owner->GetDefaultInstance()->World;
		DirectX::XMFLOAT4 pos{};
		Position = instance->Position;
		Rotation = instance->Rotation;
		Scale = instance->Scale;
		MarkDirty();
	}
}

void OLightComponent::Init(ORenderItem* Other)
{
	OSceneComponent::Init(Other);
}

DirectX::XMVECTOR OLightComponent::GetGlobalPosition() const
{
	return Load(Position);
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
void ODirectionalLightComponent::Tick(UpdateEventArgs Arg)
{
	OLightComponent::Tick(Arg);
	MarkDirty();
}

void ODirectionalLightComponent::SetCSM(OCSM* InCSM)
{
	CSM = InCSM;
	for (size_t i = 0; i < 3; i++)
	{
		DirectionalLight.ShadowMapData[i].ShadowMapIndex = CSM->GetShadowMap(i)->GetShadowMapIndex();
	}
}

OCSM* ODirectionalLightComponent::GetCSM() const
{
	return CSM;
}

void ODirectionalLightComponent::SetCascadeLambda(float InLambda)
{
	CascadeSplitLambda = InLambda;
	UpdateCascadeSplits();
	MarkDirty();
}

float ODirectionalLightComponent::GetCascadeLambda() const
{
	return CascadeSplitLambda;
}

void ODirectionalLightComponent::UpdateCascadeSplits()
{
	const auto camera = OEngine::Get()->GetWindow()->GetCamera();
	const float nearClip = camera->GetNearZ();
	const float farClip = camera->GetFarZ();
	const float clipRange = farClip - nearClip;
	const float minZ = nearClip;
	const float maxZ = nearClip + clipRange;
	const float range = maxZ - minZ;
	const float ratio = maxZ / minZ;

	// Calculate splits based on nvidia paper (https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html)
	for (uint8_t it = 0; it < MAX_CSM_PER_FRAME; it++)
	{
		float p = (it + 1) / static_cast<float>(MAX_CSM_PER_FRAME);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = CascadeSplitLambda * (log - uniform) + uniform;
		CascadeSplits[it] = (d - nearClip) / clipRange;
	}
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
	SpotLight.Intensity = Light.Intensity;
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
	const auto camera = OEngine::Get()->GetWindow()->GetCamera();

	UpdateCascadeSplits();
}

void ODirectionalLightComponent::SetLightSourceData()
{
	PROFILE_SCOPE()

	if (CSM == nullptr)
	{
		LOG(Render, Error, "CSM is nullptr");
	}

	using namespace DirectX;

	auto zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto camera = OEngine::Get()->GetWindow()->GetCamera();

	auto lightDir = XMVector3Normalize(XMLoadFloat3(&DirectionalLight.Direction));
	auto lightPos = lightDir;

	const auto cameraView = camera->GetView();
	const auto cameraProj = camera->GetProj();
	const auto viewProj = cameraView * cameraProj;

	array<XMFLOAT3, 8> arrayedCorners;
	for (int i = 0; i < 8; i++)
	{
		arrayedCorners[i] = HLSL::FrustumCornens[i];
	}

	//Calculate shadow matrices, view and proj for light based on the splits
	float lastSplitDist = 0.0f;
	for (int i = 0; i < MAX_CSM_PER_FRAME; i++)
	{
		auto frustrumCorners = arrayedCorners;
		float splitDist = CascadeSplits[i];

		//project frustrum corners into world space
		auto invViewProj = Inverse(viewProj);

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

		float texelSize = 1.0f / static_cast<float>(SRenderConstants::ShadowMapSize);
		float scale = radius * 2.0f;
		scale /= texelSize;
		scale = std::ceil(scale);
		radius = scale * texelSize / 2.0f;
		const auto eye = center + (lightDir * -radius);
		const auto lightView = MatrixLookAt(eye, center, lightUp);

		float depth = radius * RadiusScale;

		const auto lightProj = MatrixOrthographicOffCenter(-radius, radius, -radius, radius, -depth, depth);

		BoundingOrientedBox obj;
		Put(obj.Center, center);
		obj.Extents = XMFLOAT3{ radius, radius, depth }; // draw debug geometry
		Put(obj.Orientation, XMQuaternionRotationMatrix(lightView));

		OEngine::Get()->DrawDebugBox(obj.Center, obj.Extents, obj.Orientation, SColor::Blue, 0.1);

		OBoundingOrientedBox bound;
		bound.ConstructFromGeometry(obj);
		CSM->GetShadowMap(i)->UpdateBoundingGeometry(&bound, lightView);

		const XMMATRIX T{
			0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f
		};

		const auto lightViewProj = lightView * lightProj;
		const auto invLightViewProj = Inverse(lightViewProj);
		SPassConstants passConstants;
		Put(passConstants.View, Transpose(lightView));
		Put(passConstants.Proj, Transpose(lightProj));
		Put(passConstants.ViewProj, Transpose(lightViewProj));
		Put(passConstants.InvView, Transpose(Inverse(lightView)));
		Put(passConstants.InvProj, Transpose(Inverse(lightProj)));
		Put(passConstants.InvViewProj, Transpose(invLightViewProj));
		Put(passConstants.EyePosW, lightPos);

		const XMMATRIX S = lightViewProj * T;
		Put(DirectionalLight.ShadowMapData[i].Transform, Transpose(S));

		CSM->GetShadowMap(i)->SetPassConstants(passConstants);
		lastSplitDist = CascadeSplits[i];
	}
}

void OSpotLightComponent::SetLightSourceData()
{
	using namespace DirectX;

	const XMMATRIX T{
		0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f
	};

	auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Calculate target point using direction
	auto globalPos = GetGlobalPosition();
	XMVECTOR lightPos = globalPos;
	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&SpotLight.Direction));
	XMVECTOR lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction
	auto nearZ = SpotLight.FalloffStart;
	auto farZ = SpotLight.FalloffEnd;

	// Create the view matrix for the spotlight
	const auto view = MatrixLookAt(lightPos, lightTarget, lightUp);
	const auto projection = MatrixPerspective(XMConvertToRadians(SpotLight.ConeAngle * 2), 1.0f, nearZ, farZ);
	SPassConstants passConstants;
	const XMMATRIX S = view * projection * T;
	const auto invView = Inverse(view);
	const auto invProj = Inverse(projection);
	const auto viewProj = view * projection;
	const auto invViewProj = Inverse(viewProj);
	Put(passConstants.View, Transpose(view));
	Put(passConstants.Proj, Transpose(projection));
	Put(SpotLight.Transform, Transpose(S));
	Put(passConstants.View, Transpose(view));
	Put(passConstants.InvView, Transpose(invView));
	Put(passConstants.Proj, Transpose(projection));
	Put(passConstants.InvProj, Transpose(invProj));
	Put(passConstants.ViewProj, Transpose(viewProj));
	Put(passConstants.InvViewProj, Transpose(invViewProj));
	Put(passConstants.EyePosW, lightPos);
	passConstants.NearZ = nearZ;
	passConstants.FarZ = farZ;
	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, projection);
	OBoundingFrustum boundingFrustum{ frustum };
	ShadowMap->UpdateBoundingGeometry(&boundingFrustum, view);
	ShadowMap->SetPassConstants(passConstants);
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
