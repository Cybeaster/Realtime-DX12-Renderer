#pragma once
#include <filesystem>
struct SPath
{
	void SetPath(const std::wstring& Other)
	{
		Path = Other;
	}

	void SetPath(const std::filesystem::path& Other)
	{
		Path = Other;
	}

	std::string GetPath() const
	{
		return Path.string() + "/";
	}

	std::filesystem::path Path;
};