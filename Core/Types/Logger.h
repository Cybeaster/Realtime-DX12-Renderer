#pragma once

#include "DirectX/DXHelper.h"
#include "Profiler.h"

#include <boost/uuid/uuid.hpp>
#include <fstream>
#include <iostream>

#ifndef DEBUG
#define DEBUG 0
#endif

#if defined(TEXT)
#undef TEXT
#endif

enum class ELogType
{
	Log,
	Warning,
	Error,
	Critical
};

#define LOG_CATEGORY inline static const std::wstring
struct SLogCategories
{
	LOG_CATEGORY Default = L"Default";
	LOG_CATEGORY Render = L"Render";
	LOG_CATEGORY Widget = L"Widgets";
	LOG_CATEGORY Debug = L"Debug";
	LOG_CATEGORY Engine = L"Engine";
	LOG_CATEGORY Test = L"Test";
	LOG_CATEGORY Input = L"Input";
	LOG_CATEGORY Geometry = L"Geometry";
	LOG_CATEGORY Material = L"Material";
	LOG_CATEGORY Light = L"Light";
	LOG_CATEGORY Camera = L"Camera";
	LOG_CATEGORY Audio = L"Audio";
	LOG_CATEGORY Physics = L"Physics";
	LOG_CATEGORY Animation = L"Animations";
	LOG_CATEGORY Config = L"Config";
	LOG_CATEGORY TinyObjLoader = L"TinyObjLoader";
};
#define CWIN_LOG(Condition, Category, LogType, String, ...)                           \
	if (Condition)                                                                    \
	{                                                                                 \
		wstring _string_ = SLogUtils::Format(L##String, __VA_ARGS__);                 \
		SLogUtils::Log(SLogCategories::Category, _string_, ELogType::LogType, false); \
		MessageBox(0, _string_.c_str(), SLogCategories::Category.c_str(), 0);         \
	}

#define WIN_LOG(Category, LogType, String, ...)                                       \
	{                                                                                 \
		wstring _string_ = SLogUtils::Format(L##String, __VA_ARGS__);                 \
		SLogUtils::Log(SLogCategories::Category, _string_, ELogType::LogType, false); \
		MessageBox(0, _string_.c_str(), SLogCategories::Category.c_str(), 0);         \
	}

#define LOG(Category, LogType, String, ...) \
	SLogUtils::Log(SLogCategories::Category, SLogUtils::Format(L##String, ##__VA_ARGS__), ELogType::LogType, false);

#define DLOG(LogType, String, ...) \
	SLogUtils::Log(SLogUtils::Format(String, ##__VA_ARGS__), ELogType::LogType, true);

#define TEXT(Argument) \
	ToString(Argument)

struct SLogUtils
{
	static bool bLogToConsole;

	static inline map<wstring, bool> LogCategories = {
		{ SLogCategories::Default, true },
		{ SLogCategories::Render, true },
		{ SLogCategories::Widget, true },
		{ SLogCategories::Debug, true },
		{ SLogCategories::Test, false },
		{ SLogCategories::Engine, true },
		{ SLogCategories::Input, true },
		{ SLogCategories::Config, true }
	};

	static void AddCategory(wstring Category)
	{
		LogCategories.insert({ Category, true });
	}

	static void Log(wstring Category, const wstring& String, ELogType Type = ELogType::Log, const bool Debug = false) noexcept
	{
		PROFILE_SCOPE();
		if (!bLogToConsole)
		{
			return;
		}

		if (!LogCategories.contains(Category))
		{
			LogCategories.insert({ Category, true });
		}

		if ((DEBUG == false && Debug == true) || LogCategories[Category] == false)
		{
			return;
		}
		switch (Type)
		{
		case ELogType::Log:
			std::wcout << L"\033[90m" // Gray
			           << L"Log: " << String.c_str() << L"\033[0m" << std::endl;
			break;

		case ELogType::Warning:
			std::wcout << L"\033[93m" // Yellow
			           << L"Warning: " << String.c_str() << L"\033[0m" << std::endl;
			break;

		case ELogType::Error:
			std::wcout << "\n"
			           << L"\033[31m" // White text on Red background
			           << L"\t\tError: " << String.c_str() << L"\033[0m" << std::endl;
			break;

		case ELogType::Critical:
			std::wcout << "\n"
			           << L"\033[31m" // Dark Red (closest to Burgundy in basic ANSI)
			           << L"\t\tCritical: " << String.c_str() << L"\033[0m" << std::endl;
			__debugbreak();
			break;
		}
	}

	template<typename... ArgTypes>
	static std::wstring Format(std::wstring_view Str, ArgTypes&&... Args)
	{
		try
		{
			return std::vformat(Str, std::make_wformat_args(std::forward<ArgTypes>(Args)...));
		}
		catch (const std::format_error& error)
		{
			const int length = MultiByteToWideChar(CP_UTF8, 0, error.what(), -1, nullptr, 0);
			std::wstring wideErrorMessage(length, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, error.what(), -1, &wideErrorMessage[0], length);

			return wideErrorMessage + std::wstring(Str.begin(), Str.end());
		}
	}

	template<typename... ArgTypes>
	static void Printf(const std::string& Str, ArgTypes&&... Args) noexcept
	{
		std::printf(Str.c_str(), ToCString(Args)...);
	}
};

inline bool SLogUtils::bLogToConsole = true;

inline std::wstring ToString(int Argument) noexcept
{
	return std::to_wstring(Argument);
}

inline std::wstring ToString(float Argument) noexcept
{
	return std::to_wstring(Argument);
}

inline std::wstring ToString(unsigned long long Argument) noexcept
{
	return std::to_wstring(Argument);
}

inline std::wstring ToString(const char* Argument) noexcept
{
	return std::wstring(Argument, Argument + strlen(Argument));
}

inline std::wstring ToString(double Argument) noexcept
{
	return std::to_wstring(Argument);
}

inline std::wstring ToString(DirectX::XMFLOAT3 Argument) noexcept
{
	return L"[ X: " + std::to_wstring(Argument.x) + L" Y: " + std::to_wstring(Argument.y) + L" Z: " + std::to_wstring(Argument.z) + L" ]";
}

inline std::wstring ToString(string Argument) noexcept
{
	return std::wstring(Argument.begin(), Argument.end());
}

inline std::wstring ToString(boost::uuids::uuid ID) noexcept
{
	return wstring(ID.begin(), ID.end());
}

inline std::wstring ToString(const UINT Value)
{
	return std::to_wstring(Value);
}

inline std::wstring ToString(const long Value)
{
	return std::to_wstring(Value);
}

inline wstring ToString(const D3D12_DESCRIPTOR_RANGE1& Id)
{
	wstring result;
	result += L"RangeType: " + ToString(Id.RangeType) + L"\n";
	result += L"NumDescriptors: " + ToString(Id.NumDescriptors) + L"\n";
	result += L"BaseShaderRegister: " + ToString(Id.BaseShaderRegister) + L"\n";
	result += L"RegisterSpace: " + ToString(Id.RegisterSpace) + L"\n";
	result += L"OffsetInDescriptorsFromTableStart: " + ToString(Id.OffsetInDescriptorsFromTableStart) + L"\n";
	return result;
}

inline wstring ToString(UINT NumDescriptorRanges, const D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges)
{
	wstring result;
	for (UINT i = 0; i < NumDescriptorRanges; i++)
	{
		result += L"\t";
		result += ToString(pDescriptorRanges[i]);
	}
	return result;
}

inline wstring ToString(D3D12_ROOT_DESCRIPTOR_TABLE1 Id)
{
	wstring result;
	result += L"NumDescriptorRanges: " + ToString(Id.NumDescriptorRanges) + L"\n";
	result += L"pDescriptorRanges: " + ToString(Id.NumDescriptorRanges, Id.pDescriptorRanges) + L"\n";

	return result;
}

inline wstring ToString(const D3D12_ROOT_PARAMETER1& Id)
{
	wstring result;
	result += L"ParameterType: " + ToString(Id.ParameterType) + L"\n";
	result += L"ShaderVisibility: " + ToString(Id.ShaderVisibility) + L"\n";
	result += L"DescriptorTable: " + ToString(Id.DescriptorTable) + L"\n";
	return result;
}

inline wstring ToString(const D3D12_ROOT_PARAMETER_TYPE& Id)
{
	switch (Id)
	{
	case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		return L"Descriptor Table";
		break;
	case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		return L"32 Bit Constants";
		break;
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		return L"Constant Buffer View";
		break;
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
		return L"Shader Resource View";
		break;
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		return L"Unordered Access View";
		break;
	}
	return L"Unknown";
}

inline wstring ToString(const DirectX::XMVECTOR& Vec)
{
	return L"[ X: " + ToString(DirectX::XMVectorGetX(Vec)) + L" Y: " + ToString(DirectX::XMVectorGetY(Vec)) + L" Z: " + ToString(DirectX::XMVectorGetZ(Vec)) + L" W: " + ToString(DirectX::XMVectorGetW(Vec)) + L" ]";
}

inline wstring ToString(const DirectX::XMFLOAT4 Vec)
{
	return L"[ X: " + ToString(Vec.x) + L" Y: " + ToString(Vec.y) + L" Z: " + ToString(Vec.z) + L" W: " + ToString(Vec.w) + L" ]";
}

inline wstring ToString(const D3D12_SHADER_VISIBILITY& Id)
{
	switch (Id)
	{
	case D3D12_SHADER_VISIBILITY_ALL:
		return L"All";
		break;
	case D3D12_SHADER_VISIBILITY_VERTEX:
		return L"Vertex";
		break;
	case D3D12_SHADER_VISIBILITY_HULL:
		return L"Hull";
		break;
	case D3D12_SHADER_VISIBILITY_DOMAIN:
		return L"Domain";
		break;
	case D3D12_SHADER_VISIBILITY_GEOMETRY:
		return L"Geometry";
		break;
	case D3D12_SHADER_VISIBILITY_PIXEL:
		return L"Pixel";
		break;
	case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
		return L"Amplification";
		break;
	case D3D12_SHADER_VISIBILITY_MESH:
		return L"Mesh";
		break;
	}
	return L"Unknown";
}

inline wstring ToString(D3D12_TEXTURE_ADDRESS_MODE Mode)
{
	switch (Mode)
	{
	case D3D12_TEXTURE_ADDRESS_MODE_WRAP:
		return L"Wrap";
		break;
	case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:
		return L"Mirror";
		break;
	case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:
		return L"Clamp";
		break;
	case D3D12_TEXTURE_ADDRESS_MODE_BORDER:
		return L"Border";
		break;
	case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:
		return L"Mirror Once";
		break;
	}
	return L"Unknown";
}

inline wstring ToString(D3D12_FILTER Other)
{
	switch (Other)
	{
	case D3D12_FILTER_MIN_MAG_MIP_POINT:
		return L"Min Mag Mip Point";
		break;
	case D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		return L"Min Mag Point Mip Linear";
		break;
	case D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return L"Min Point Mag Linear Mip Point";
		break;
	case D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		return L"Min Point Mag Mip Linear";
		break;
	case D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		return L"Min Linear Mag Mip Point";
		break;
	case D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return L"Min Linear Mag Point Mip Linear";
		break;
	case D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		return L"Min Mag Linear Mip Point";
		break;
	case D3D12_FILTER_MIN_MAG_MIP_LINEAR:
		return L"Min Mag Mip Linear";
		break;
	case D3D12_FILTER_ANISOTROPIC:
		return L"Anisotropic";
		break;
	case D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT:
		return L"Comparison Min Mag Mip Point";
		break;
	case D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
		return L"Comparison Min Mag Point Mip Linear";
		break;
	case D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return L"Comparison Min Point Mag Linear Mip Point";
		break;
	case D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
		return L"Comparison Min Point Mag Mip Linear";
		break;
	case D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
		return L"Comparison Min Linear Mag Mip Point";
		break;
	case D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return L"Comparison Min Linear Mag Point Mip Linear";
		break;
	case D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
		return L"Comparison Min Mag Linear Mip Point";
		break;
	case D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR:
		return L"Comparison Min Mag Mip Linear";
		break;
	case D3D12_FILTER_COMPARISON_ANISOTROPIC:
		return L"Comparison Anisotropic";
		break;
	case D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT:
		return L"Minimum Min Mag Mip Point";
		break;
	case D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return L"Minimum Min Mag Point Mip Linear";
		break;
	case D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return L"Minimum Min Point Mag Linear Mip Point";
		break;
	case D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return L"Minimum Min Point Mag Mip Linear";
		break;
	case D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return L"Minimum Min Linear Mag Mip Point";
		break;
	case D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return L"Minimum Min Linear Mag Point Mip Linear";
		break;
	case D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
		return L"Minimum Min Mag Linear Mip Point";
		break;
	case D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR:
		return L"Minimum Min Mag Mip Linear";
		break;
	case D3D12_FILTER_MINIMUM_ANISOTROPIC:
		return L"Minimum Anisotropic";
		break;
	case D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT:
		return L"Maximum Min Mag Mip Point";
		break;
	case D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return L"Maximum Min Mag Point Mip Linear";
		break;
	case D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return L"Maximum Min Point Mag Linear Mip Point";
		break;
	case D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return L"Maximum Min Point Mag Mip Linear";
		break;
	case D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return L"Maximum Min Linear Mag Mip Point";
		break;
	}
}

inline wstring ToString(const D3D12_STATIC_SAMPLER_DESC& Id)
{
	wstring result;
	result += L"Filter: " + ToString(Id.Filter) + L"\n";
	result += L"AddressU: " + ToString(Id.AddressU) + L"\n";
	result += L"AddressV: " + ToString(Id.AddressV) + L"\n";
	result += L"AddressW: " + ToString(Id.AddressW) + L"\n";
	result += L"MipLODBias: " + ToString(Id.MipLODBias) + L"\n";
	result += L"MaxAnisotropy: " + ToString(Id.MaxAnisotropy) + L"\n";
	result += L"ComparisonFunc: " + ToString(Id.ComparisonFunc) + L"\n";
	result += L"BorderColor: " + ToString(Id.BorderColor) + L"\n";
	result += L"MinLOD: " + ToString(Id.MinLOD) + L"\n";
	result += L"MaxLOD: " + ToString(Id.MaxLOD) + L"\n";
	result += L"ShaderRegister: " + ToString(Id.ShaderRegister) + L"\n";
	result += L"RegisterSpace: " + ToString(Id.RegisterSpace) + L"\n";
	result += L"ShaderVisibility: " + ToString(Id.ShaderVisibility) + L"\n";
	return result;
}

template<typename T>
struct SLogger
{
	SLogger() = default;

	explicit SLogger(std::shared_ptr<T> S)
	    : Stream(S)
	{
	}

	void Log(const std::string& Message) const
	{
		auto stream = Stream.get();
		*stream << Message << "\n"
		        << std::flush;
	}

	void SetStream(std::shared_ptr<T> S)
	{
		Stream = S;
	}

	~SLogger()
	{
		Stream->close();
	}

private:
	std::shared_ptr<T> Stream;
};
