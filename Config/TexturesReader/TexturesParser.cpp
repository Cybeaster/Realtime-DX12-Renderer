//
// Created by Cybea on 24/02/2024.
//

#include "TexturesParser.h"

#include "../../Textures/Texture.h"

#include <ranges>

void OTexturesParser::AddTexture(STexture* Texture)
{
	auto& items = PTree.get_child("Textures");
	auto name = Texture->Name;
	auto path = WStringToUTF8(Texture->FileName);
	for (auto& val : items | std::views::values)
	{
		if (val.get<string>("Name") == name)
		{
			auto& texNode = val;
			texNode.put("Path", path);

			return;
		}
	}
}

void OTexturesParser::AddTextures(vector<STexture*> Textures)
{
}

vector<STexture*> OTexturesParser::LoadTextures()
{
}