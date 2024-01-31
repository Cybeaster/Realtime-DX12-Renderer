#pragma once
#include "Exception.h"
#include "Types.h"

#include <Windows.h>

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

struct SLogCategories
{
	inline static const std::string Default = "Default";
	inline static const std::string Render = "Render";
	inline static const std::string Widget = "Widgets";
	inline static const std::string Debug = "Debug";
	inline static const std::string Engine = "Engine";
	inline static const std::string Test = "Test";
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
		{ SLogCategories::Engine, true }
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
