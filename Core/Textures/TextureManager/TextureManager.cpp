#include "TextureManager.h"

#include "Application.h"
#include "CommandQueue/CommandQueue.h"
#include "DDSTextureLoader/DDSTextureLoader.h"
#include "Exception.h"
#include "Logger.h"

#include <filesystem>
#include <numeric>
#include <ranges>
#include <unordered_set>

OTextureManager::OTextureManager(ID3D12Device* Device, OCommandQueue* Queue)
    : Device(Device), CommandQueue(Queue)
{
	Parser = make_unique<OTexturesParser>(OApplication::Get()->GetConfigPath("TexturesConfigPath"));
	LoadLocalTextures();
}

uint32_t OTextureManager::GetNum2DTextures() const
{
	uint32_t acc = 0;
	for (const auto& texture : Textures | std::views::values)
	{
		if (texture->ViewType == STextureViewType::Texture2D)
		{
			acc++;
		}
	}
	return acc;
}

uint32_t OTextureManager::GetNum3DTextures() const
{
	uint32_t acc = 0;
	for (const auto& texture : Textures | std::views::values)
	{
		if (texture->ViewType == STextureViewType::TextureCube)
		{
			acc++;
		}
	}
	return acc;
}

uint32_t OTextureManager::GetNumTotalTextures() const
{
	return Textures.size();
}

void OTextureManager::LoadLocalTextures()
{
	CommandQueue->TryResetCommandList();
	RemoveAllTextures();

	for (const auto& texture : Parser->LoadTextures())
	{
		texture->HeapIdx = Textures.size();
		TexturesHeapIndicesTable.insert(texture->HeapIdx);
		THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		                                                    CommandQueue->GetCommandList().Get(),
		                                                    OApplication::Get()->GetResourcePath(texture->FileName).c_str(),
		                                                    texture->Resource.Resource,
		                                                    texture->UploadHeap.Resource));
		texture->Resource.Init(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		texture->Resource.Resource->SetName(texture->FileName.c_str());

		CommandQueue->ResourceBarrier(&texture->Resource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		LOG(Engine, Log, "Texture created from config: Name : {}, Path: {}", TEXT(texture->Name), texture->FileName);
		AddTexture(make_unique<STexture>(*texture));
	}
	CommandQueue->ExecuteCommandListAndWait();
}

void OTextureManager::RemoveAllTextures()
{
	TexturesHeapIndicesTable.clear();
	Textures.clear();
	TexturesPath.clear();
}

void OTextureManager::SaveLocalTextures()
{
	vector<STexture*> textures;
	for (auto& texture : Textures | std::views::values)
	{
		textures.push_back(texture.get());
	}
	Parser->AddTextures(textures);
}

wstring OTextureManager::GetName()
{
	return Name;
}

void OTextureManager::AddTexture(unique_ptr<STexture> Texture)
{
	if (Texture->Name.empty())
	{
		LOG(Engine, Error, "Texture name is empty!");
		return;
	}

	if (Textures.contains(Texture->Name))
	{
		LOG(Engine, Error, "Texture with this name already exists!");
		return;
	}

	TexturesPath[Texture->FileName] = Texture.get();
	Textures[Texture->Name] = move(Texture);
}

void OTextureManager::RemoveTexture(const string& Name)
{
	if (!Textures.contains(Name))
	{
		return;
	}
	auto texture = Textures.at(Name).get();
	TexturesPath.erase(texture->FileName);
	Textures.erase(Name);
}

void OTextureManager::RemoveTexture(const wstring& Path)
{
	if (!TexturesPath.contains(Path))
	{
		return;
	}
	auto texture = TexturesPath.at(Path);
	TexturesPath.erase(Path);
	Textures.erase(texture->Name);
}

STexture* OTextureManager::CreateTexture(const string& Name, wstring FileName)
{
	auto name = Name;
	if (Textures.contains(name))
	{
		if (TexturesPath.contains(FileName))
		{
			LOG(Engine, Warning, "Texture with this name already exists!");
			return Textures[name].get();
		}
		name = name + "_" + std::to_string(Textures.size());
	}

	std::filesystem::path path(FileName);
	if (!exists(path))
	{
		LOG(Engine, Warning, "Texture file not found!");
		return nullptr;
	}

	auto texture = make_unique<STexture>();
	texture->Name = name;
	texture->FileName = FileName;
	texture->HeapIdx = Textures.size();
	CWIN_LOG(TexturesHeapIndicesTable.contains(texture->HeapIdx), Engine, Error, "Texture heap index already exists! {}", texture->HeapIdx);
	CWIN_LOG(texture->HeapIdx > TEXTURE_MAPS_NUM, Engine, Error, "Texture index is out of range!");
	TexturesHeapIndicesTable.insert(texture->HeapIdx);

	auto res = DirectX::LoadTexture(FileName, Device, CommandQueue->GetCommandList().Get(), texture->Resource.Resource, texture->UploadHeap.Resource, path.extension() == ".dds");
	CWIN_LOG(!SUCCEEDED(res), Engine, Error, "Failed to load texture from file: {}", FileName);
	texture->Resource.Init(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	texture->Resource.Resource->SetName(path.filename().c_str());
	if (texture->Type == ETextureType::Height)
	{
		CommandQueue->ResourceBarrier(&texture->Resource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	LOG(Engine, Log, "Texture created: Name : {}, Path: {}", TEXT(name), FileName);

	auto result = texture.get();
	AddTexture(std::move(texture));
	return result;
}

STexture* OTextureManager::CreateTexture(const wstring& FileName)
{
	auto path = std::filesystem::path(FileName);
	return CreateTexture(path.filename().stem().string(), FileName);
}

STexture* OTextureManager::FindTextureByName(string Name) const
{
	if (!Textures.contains(Name))
	{
		LOG(Engine, Warning, "Texture not found! Name: {}", TEXT(Name));
		return nullptr;
	}
	return Textures.at(Name).get();
}

STexture* OTextureManager::FindTextureByPath(wstring Path) const
{
	if (!TexturesPath.contains(Path))
	{
		return nullptr;
	}
	return TexturesPath.at(Path);
}

STexture* OTextureManager::FindOrCreateTexture(wstring FileName)
{
	if (auto texture = FindTextureByPath(FileName); texture)
	{
		return texture;
	}
	return CreateTexture(FileName);
}

STexture* OTextureManager::FindOrCreateTexture(string Name, wstring FileName)
{
	if (auto texture = FindTextureByPath(FileName); texture)
	{
		return texture;
	}
	return CreateTexture(Name, FileName);
}
