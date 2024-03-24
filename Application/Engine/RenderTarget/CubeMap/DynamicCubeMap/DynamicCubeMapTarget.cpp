
#include "DynamicCubeMapTarget.h"

#include "Engine/Engine.h"

void ODynamicCubeMapRenderTarget::InitRenderObject()
{
	using namespace DirectX;
	OCubeRenderTarget::InitRenderObject();

	auto x = Position.x;
	auto y = Position.y;
	auto z = Position.z;

	XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] = {
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y, z + 1.0f), // +Z
		XMFLOAT3(x, y, z - 1.0f) // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] = {
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f), // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f) // -Z
	};
	Cameras.resize(GetNumRTVRequired());
	for (int i = 0; i < GetNumRTVRequired(); ++i)
	{
		Cameras[i] = make_unique<OCamera>();
		Cameras[i]->LookAt(Position, targets[i], ups[i]);
		Cameras[i]->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		Cameras[i]->UpdateViewMatrix();
	}
}
void ODynamicCubeMapRenderTarget::UpdatePass(const TUploadBufferData<SPassConstants>& Data)
{
	OCubeRenderTarget::UpdatePass(Data);
	auto start = Data.StartIndex;
	auto end = Data.EndIndex;
	PassConstants.clear();
	for (; start < end; start++)
	{
		auto norm = start - Data.StartIndex;
		auto pass = OEngine::Get()->GetMainPassCB();
		auto camera = Cameras[norm].get();
		camera->FillPassConstant(pass);
		pass.RenderTargetSize = DirectX::XMFLOAT2(Resolution.x, Resolution.y);
		pass.InvRenderTargetSize = DirectX::XMFLOAT2(1 / SCast<float>(Resolution.x), SCast<float>(1 / Resolution.y));
		Data.Buffer->CopyData(start, pass);

		TUploadBufferData<SPassConstants> data;
		data.StartIndex = start;
		data.EndIndex = end;
		data.Buffer = Data.Buffer;
		PassConstants.push_back(data);
	}
}