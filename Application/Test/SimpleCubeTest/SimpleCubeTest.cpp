
#include "SimpleCubeTest.h"

#include "../../Engine/Engine.h"
#include "../../Window/Window.h"
#include "Application.h"
#include "Camera/Camera.h"

#include <DXHelper.h>

#include <filesystem>
#include <iostream>
using namespace Microsoft::WRL;

using namespace DirectX;

OSimpleCubeTest::OSimpleCubeTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
    : OTest(_Engine, _Window)
{
}

void OSimpleCubeTest::LoadContent()
{
}

void OSimpleCubeTest::UnloadContent()
{
	ContentLoaded = false;
}

void OSimpleCubeTest::OnUpdate(const UpdateEventArgs& Event)
{
	Super::OnUpdate(Event);
	auto window = Window.lock();
}

void OSimpleCubeTest::OnRender(const UpdateEventArgs& Event)
{
	const auto window = Window.lock();
	const auto camera = window->GetCamera().get();
	float x = Radius * sinf(Phi) * cosf(Theta);
	float z = Radius * sinf(Phi) * sinf(Theta);
	float y = Radius * cosf(Phi);

	const XMMATRIX world = XMLoadFloat4x4(&ModelMatix);
	const XMMATRIX worldViewProj = world * camera->ViewMatrix * camera->ProjectionMatrix;

	SObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.ModelViewProj, XMMatrixTranspose(worldViewProj));
	ObjectCB->CopyData(0, objConstants);
}

void OSimpleCubeTest::OnResize(const ResizeEventArgs& Event)
{
	OTest::OnResize(Event);
}

void OSimpleCubeTest::OnMouseWheel(const MouseWheelEventArgs& Event)
{
	const auto window = Window.lock();
	auto fov = window->GetFoV();
	fov -= Event.WheelDelta;
	fov = std::clamp(fov, 12.0f, 90.0f);
	window->SetFoV(fov);
	char buffer[256];
	sprintf_s(buffer, "FoV: %f\n", fov);
	OutputDebugStringA(buffer);
}

void OSimpleCubeTest::OnKeyPressed(const KeyEventArgs& Event)
{
	OTest::OnKeyPressed(Event);

	switch (Event.Key)
	{
	case KeyCode::Escape:
		OApplication::Get()->Quit(0);
		break;
	case KeyCode::Enter:
		if (Event.Alt)
		{
		case KeyCode::F11:
			Engine.lock()->GetWindow()->ToggleFullscreen();
			break;
		}
	case KeyCode::V:
		Engine.lock()->GetWindow()->ToggleVSync();
		break;
	}
}
void OSimpleCubeTest::OnMouseMoved(const MouseMotionEventArgs& Args)
{
	OTest::OnMouseMoved(Args);
	auto window = Window.lock();

	if (Args.LeftButton)
	{
		float dx = XMConvertToRadians(0.25f * (Args.X - window->GetLastXMousePos()));
		float dy = XMConvertToRadians(0.25f * (Args.Y - window->GetLastYMousePos()));

		Theta += dx;
		Phi += dy;

		Phi = std::clamp(Phi, 0.1f, XM_PI - 0.1f);
	}

	else if (Args.RightButton)
	{
		float dx = 0.005f * (Args.X - window->GetLastXMousePos());
		float dy = 0.005f * (Args.Y - window->GetLastYMousePos());
		Radius += dx - dy;

		Radius = std::clamp(Radius, 3.0f, 15.0f);
	}
}

void OSimpleCubeTest::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> CommandList,
                                           ID3D12Resource** DestinationResource,
                                           ID3D12Resource** IntermediateResource,
                                           size_t NumElements,
                                           size_t ElementSize,
                                           const void* BufferData,
                                           D3D12_RESOURCE_FLAGS Flags) const
{
	auto device = Engine.lock()->GetDevice();
	size_t bufferSize = NumElements * ElementSize;

	// Create named instances for heap properties and resource description
	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, Flags);

	// Create a committed resource for the GPU resource in a default heap.
	THROW_IF_FAILED(device->CreateCommittedResource(
	    &defaultHeapProperties,
	    D3D12_HEAP_FLAG_NONE,
	    &bufferResourceDesc,
	    D3D12_RESOURCE_STATE_COMMON,
	    nullptr,
	    IID_PPV_ARGS(DestinationResource)));

	if (BufferData)
	{
		// Create named instances for upload heap properties and resource description
		const CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC uploadBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		// Create a committed resource for the upload.
		THROW_IF_FAILED(device->CreateCommittedResource(
		    &uploadHeapProperties,
		    D3D12_HEAP_FLAG_NONE,
		    &uploadBufferResourceDesc,
		    D3D12_RESOURCE_STATE_GENERIC_READ,
		    nullptr,
		    IID_PPV_ARGS(IntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = BufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(CommandList.Get(), *DestinationResource, *IntermediateResource, 0, 0, 1, &subresourceData);
	}
}

void OSimpleCubeTest::ResizeDepthBuffer(int Width, int Height)
{
	if (ContentLoaded)
	{
	}
}

void OSimpleCubeTest::CompileShader(const WCHAR* FileName, const char* EntryPoint, const char* Target, ID3DBlob** Blob) const
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
	HRESULT hr = D3DCompileFromFile(FileName, nullptr, nullptr, EntryPoint, Target, dwShaderFlags, 0, Blob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		}
		THROW_IF_FAILED(hr);
	}
}
