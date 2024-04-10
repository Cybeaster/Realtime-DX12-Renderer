#pragma once
#include "Application.h"

#include <Types.h>

namespace Utils
{
inline unique_ptr<std::fstream> OpenFile(const string& FileName, const std::ios_base::openmode Mode = std::ios_base::in)
{
	auto fIn = make_unique<std::fstream>(FileName, Mode);
	if (!fIn->is_open())
	{
		const wstring fileName = wstring(FileName.begin(), FileName.end());
		const wstring error = L"Failed to open file: " + fileName;
		MessageBox(nullptr, error.c_str(), L"Error", MB_OK);
	}
	return move(fIn);
}
} // namespace Utils

inline wstring ToString(const std::filesystem::path& Path)
{
	return Path.wstring();
}
