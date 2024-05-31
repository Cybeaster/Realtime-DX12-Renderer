#pragma once
#include "DirectX/DXHelper.h"

struct STransform
{
	STransform() = default;
	STransform(const DirectX::XMVECTOR& InPosition, const DirectX::XMVECTOR& InRotation, const DirectX::XMVECTOR& InScale)
	    : Position(InPosition), Rotation(InRotation), Scale(InScale)
	{
	}

	STransform(const DirectX::XMFLOAT3& InPosition, const DirectX::XMFLOAT3& InRotation, const DirectX::XMFLOAT3& InScale)
	    : Position(DirectX::XMLoadFloat3(&InPosition)), Rotation(DirectX::XMLoadFloat3(&InRotation)), Scale(DirectX::XMLoadFloat3(&InScale))
	{
	}

	STransform(const DirectX::XMFLOAT3& InPosition, const DirectX::XMFLOAT4& InRotation, const DirectX::XMFLOAT3& InScale)
	    : Position(DirectX::XMLoadFloat3(&InPosition)), Rotation(DirectX::XMLoadFloat4(&InRotation)), Scale(DirectX::XMLoadFloat3(&InScale))
	{
	}

	DirectX::XMVECTOR Position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR Scale = { 1.0f, 1.0f, 1.0f };

	auto GetFloat3Rotation() const
	{
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, Rotation);
		return result;
	}

	auto GetFloat3Position() const
	{
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, Position);
		return result;
	}

	auto GetFloat3Scale() const
	{
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, Scale);
		return result;
	}

	void SetFloat3Rotation(const DirectX::XMFLOAT3& NewRotation)
	{
		Rotation = DirectX::XMLoadFloat3(&NewRotation);
	}

	void SetFloat4Rotation(const DirectX::XMFLOAT4& NewRotation)
	{
		Rotation = DirectX::XMLoadFloat4(&NewRotation);
	}

	void SetFloat3Position(const DirectX::XMFLOAT3& NewPosition)
	{
		Position = DirectX::XMLoadFloat3(&NewPosition);
	}

	void SetFloat3Scale(const DirectX::XMFLOAT3& NewScale)
	{
		Scale = DirectX::XMLoadFloat3(&NewScale);
	}

	void SetRotation(const DirectX::XMVECTOR& NewRotation)
	{
		Rotation = NewRotation;
	}

	void SetPosition(const DirectX::XMVECTOR& NewPosition)
	{
		Position = NewPosition;
	}

	void SetPosition(const DirectX::XMFLOAT3& NewPosition)
	{
		Position = DirectX::XMLoadFloat3(&NewPosition);
	}

	void SetRotation(const DirectX::XMFLOAT3& NewRotation)
	{
		Rotation = DirectX::XMLoadFloat3(&NewRotation);
	}

	void SetScale(const DirectX::XMFLOAT3& NewScale)
	{
		Scale = DirectX::XMLoadFloat3(&NewScale);
	}

	void SetScale(const DirectX::XMFLOAT4& NewScale)
	{
		Scale = DirectX::XMLoadFloat4(&NewScale);
	}

	void SetScale(const DirectX::XMVECTOR& NewScale)
	{
		Scale = NewScale;
	}
};