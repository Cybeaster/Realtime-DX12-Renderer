#pragma once
#include <format>
#include <iostream>
#include <memory>
#include <ostream>
#include <Types.h>

#ifndef DEBUG
#define DEBUG 0
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

#define LOG(LogType, String, ...) \
SLogUtils::Log(SLogUtils::Format(String, ##__VA_ARGS__), ELogType::LogType, false);

#define DLOG(LogType, String, ...) \
SLogUtils::Log(SLogUtils::Format(String, ##__VA_ARGS__), ELogType::LogType, true);


#define TO_STRING(Argument) \
	SLogUtils::ToString(Argument)

struct SLogUtils
{
	template<typename Object>
	static void Log(const Object& String, ELogType Type = ELogType::Log, const bool Debug = false) noexcept
	{
		if (DEBUG == false && Debug == true)
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
			break;

		case ELogType::Critical:
			std::clog << "\n \t \t"
				<< "Critical: " << String << std::endl;
			break;
		}
	}

	template<typename... ArgTypes>
	static string Format(std::wstring_view Str, ArgTypes&&... Args)
	{
		try
		{
			return std::vformat(Str, std::make_format_args(Args...));
		}
		catch (const std::format_error& error)
		{
			return error.what() + string(Str.begin(), Str.end());
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
	static void Printf(const string& Str, ArgTypes&&... Args) noexcept
	{
		std::printf(Str.c_str(), ToCString(Args)...);
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
		*stream << Message << "\n" << std::flush;
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
