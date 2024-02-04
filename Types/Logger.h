#pragma once
#include "Exception.h"
#include "Types.h"

#include <DirectXMath.h>
#include <Windows.h>
#include <dxgi1_3.h>

#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#ifndef DEBUG
#define DEBUG 0
#endif

#if defined(TEXT)
#undef TEXT
#endif

#define TEXT(Arg) \
	L##Arg

enum class ELogType
{
	Log,
	Warning,
	Error,
	Critical
};

#define LOG_CATEGORY inline static const std::string
struct SLogCategories
{
	LOG_CATEGORY Default = "Default";
	LOG_CATEGORY Render = "Render";
	LOG_CATEGORY Widget = "Widgets";
	LOG_CATEGORY Debug = "Debug";
	LOG_CATEGORY Engine = "Engine";
	LOG_CATEGORY Test = "Test";
	LOG_CATEGORY Input = "Input";
};

#define LOG(Category, LogType, String, ...) \
	SLogUtils::Log(SLogCategories::Category, SLogUtils::Format(String, ##__VA_ARGS__), ELogType::LogType, false);

#define DLOG(LogType, String, ...) \
	SLogUtils::Log(SLogUtils::Format(String, ##__VA_ARGS__), ELogType::LogType, true);

#define TO_STRING(Argument) \
	SLogUtils::ToString(Argument)

struct SLogUtils
{
	static inline map<string, bool> LogCategories = {
		{ SLogCategories::Default, true },
		{ SLogCategories::Render, true },
		{ SLogCategories::Widget, true },
		{ SLogCategories::Debug, false },
		{ SLogCategories::Test, false },
		{ SLogCategories::Engine, true },
		{ SLogCategories::Input, true }
	};

	static void AddCategory(string Category)
	{
		LogCategories.insert({ Category, true });
	}

	template<typename Object>
	static void Log(string Category, const Object& String, ELogType Type = ELogType::Log, const bool Debug = false) noexcept
	{
		if ((DEBUG == false && Debug == true) || LogCategories[Category] == false)
		{
			return;
		}

		switch (Type)
		{
		case ELogType::Log:
			std::cout << "\n"
			          << "Log: " << String << std::endl;
			break;

		case ELogType::Warning:
			std::clog << "\n"
			          << "Warning: " << String << std::endl;
			break;

		case ELogType::Error:
			std::clog << "\n \t \t"
			          << "Error: " << String << std::endl;
			__debugbreak();
			break;

		case ELogType::Critical:
			std::clog << "\n \t \t"
			          << "Critical: " << String << std::endl;
			assert(false);
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
	static std::string Format(std::string_view Str, ArgTypes&&... Args)
	{
		try
		{
			return std::vformat(Str, std::make_format_args(std::forward<ArgTypes>(Args)...));
		}
		catch (const std::format_error& error)
		{
			return std::string(error.what()) + std::string(Str);
		}
	}

	template<typename... ArgTypes>
	static void Printf(const std::string& Str, ArgTypes&&... Args) noexcept
	{
		std::printf(Str.c_str(), ToCString(Args)...);
	}

	static std::string ToString(int Argument) noexcept
	{
		return std::to_string(Argument);
	}

	static std::string ToString(float Argument) noexcept
	{
		return std::to_string(Argument);
	}

	static std::string ToString(double Argument) noexcept
	{
		return std::to_string(Argument);
	}

	static std::string ToString(DirectX::XMFLOAT3 Argument) noexcept
	{
		return "[ X: " + std::to_string(Argument.x) + " Y: " + std::to_string(Argument.y) + " Z: " + std::to_string(Argument.z) + " ]";
	}
};

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
