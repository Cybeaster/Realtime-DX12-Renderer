#pragma once
#include "DXHelper.h"

class IRenderObject;
struct SResourceInfo
{
	static shared_ptr<SResourceInfo> MakeResourceInfo()
	{
		return make_shared<SResourceInfo>();
	}

	static shared_ptr<SResourceInfo> MakeResourceInfo(const wstring& Name, const weak_ptr<IRenderObject>& InContext, const ComPtr<ID3D12Resource>& InResource, D3D12_RESOURCE_STATES InState)
	{
		auto info = make_shared<SResourceInfo>();
		info->Context = InContext;
		info->Resource = InResource;
		info->CurrentState = InState;
		info->Name = Name;
		InResource->SetName(Name.c_str());
		return info;
	}

	void Init(weak_ptr<IRenderObject> InContext, D3D12_RESOURCE_STATES InState)
	{
		Context = InContext;
		CurrentState = InState;
	}

	ComPtr<ID3D12Resource> operator=(const ComPtr<ID3D12Resource>& InResource)
	{
		Resource = InResource;
		return Resource;
	}

	D3D12_RESOURCE_STATES CurrentState;
	ComPtr<ID3D12Resource> Resource;
	weak_ptr<IRenderObject> Context;
	wstring Name = L"NONE";
};

using TResourceInfo = shared_ptr<SResourceInfo>;
