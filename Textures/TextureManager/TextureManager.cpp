#include "TextureManager.h"

#include "../DDSTextureLoader/DDSTextureLoader.h"
#include "Application.h"
#include "CommandQueue/CommandQueue.h"
#include "Exception.h"
#include "Logger.h"
#include "Settings.h"

#include <filesystem>
#include <ranges>
#include <unordered_set>
OTextureManager::OTextureManager(ID3D12Device* Device, OCommandQueue* Queue)
    : Device(Device), CommandQueue(Queue)
{
	Parser = make_unique<OTexturesParser>(OApplication::Get()->GetConfigPath("TexturesConfigPath"));
	LoadLocalTextures();
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
		                                                    texture->FileName.c_str(),
		                                                    texture->Resource,
		                                                    texture->UploadHeap));
		LOG(Engine, Log, "Texture created from config: Name : {}, Path: {}", TO_STRING(texture->Name), texture->FileName);
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

void OTextureManager::AddTexture(unique_ptr<STexture> Texture)
{
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

STexture* OTextureManager::CreateTexture(string Name, wstring FileName)
{
	if (Textures.contains(Name))
	{
		LOG(Engine, Error, "Texture with this name already exists!");
		return nullptr;
	}

	std::filesystem::path path(FileName);
	if (!exists(path))
	{
		LOG(Engine, Warning, "Texture file not found!");
		return nullptr;
	}

	auto texture = make_unique<STexture>();
	texture->Name = Name;
	texture->FileName = FileName;
	texture->HeapIdx = Textures.size();

	CWIN_LOG(TexturesHeapIndicesTable.contains(texture->HeapIdx), Engine, Error, "Texture heap index already exists! {}", texture->HeapIdx);
	TexturesHeapIndicesTable.insert(texture->HeapIdx);

	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
	                                                    CommandQueue->GetCommandList().Get(),
	                                                    texture->FileName.c_str(),
	                                                    texture->Resource,
	                                                    texture->UploadHeap));
	LOG(Engine, Log, "Texture created: Name : {}, Path: {}", TO_STRING(Name), FileName);

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
		LOG(Engine, Warning, "Texture not found!");
		return nullptr;
	}
	return Textures.at(Name).get();
}

STexture* OTextureManager::FindTextureByPath(wstring Path) const
{
	if (!TexturesPath.contains(Path))
	{
		LOG(Engine, Warning, "Texture not found!");
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
