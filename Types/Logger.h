#pragma once

#include "DirectX/DXHelper.h"

#include <boost/uuid/uuid.hpp>
#include <fstream>
#include <iostream>

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

#define TO_STRING(Argument) \
	SLogUtils::ToString(Argument)

struct SLogUtils
{
	static inline map<wstring, bool> LogCategories = {
		{ SLogCategories::Default, true },
		{ SLogCategories::Render, true },
		{ SLogCategories::Widget, true },
		{ SLogCategories::Debug, true },
		{ SLogCategories::Test, false },
		{ SLogCategories::Engine, true },
		{ SLogCategories::Input, true }
	};

	static void AddCategory(wstring Category)
	{
		LogCategories.insert({ Category, true });
	}

	static void Log(wstring Category, const wstring& String, ELogType Type = ELogType::Log, const bool Debug = false) noexcept
	{
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
			std::wcout << "\n"
			           << L"Log: " << String.c_str() << std::endl;
			break;

		case ELogType::Warning:
			std::wcout << "\n"
			           << L"Warning: " << String.c_str() << std::endl;
			break;

		case ELogType::Error:
			std::wcout << "\n \t \t"
			           << L"Error: " << String.c_str() << std::endl;
			__debugbreak();
			break;

		case ELogType::Critical:
			std::wcout << "\n \t \t"
			           << L"Critical: " << String.c_str() << std::endl;
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
	static void Printf(const std::string& Str, ArgTypes&&... Args) noexcept
	{
		std::printf(Str.c_str(), ToCString(Args)...);
	}

	static std::wstring ToString(int Argument) noexcept
	{
		return std::to_wstring(Argument);
	}

	static std::wstring ToString(float Argument) noexcept
	{
		return std::to_wstring(Argument);
	}

	static std::wstring ToString(double Argument) noexcept
	{
		return std::to_wstring(Argument);
	}

	static std::wstring ToString(DirectX::XMFLOAT3 Argument) noexcept
	{
		return L"[ X: " + std::to_wstring(Argument.x) + L" Y: " + std::to_wstring(Argument.y) + L" Z: " + std::to_wstring(Argument.z) + L" ]";
	}

	static std::wstring ToString(string Argument) noexcept
	{
		return std::wstring(Argument.begin(), Argument.end());
	}

	static std::wstring ToString(boost::uuids::uuid ID) noexcept
	{
		return wstring(ID.begin(), ID.end());
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
