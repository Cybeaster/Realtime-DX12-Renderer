#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Logger.h"

#include <windows.h>

#include <exception>

#define THROW_IF_FAILED(hr) ThrowIfFailed(hr, string(__FILE__) + TO_STRING(__LINE__));

inline void ThrowIfFailed(HRESULT Hr, string Message)
{
	if (FAILED(Hr))
	{
		LOG(Error, "ThrowIfFailed: {}" + Message, TO_STRING(Hr));
		throw std::exception();
	}
}

inline void ThrowIfFailed(HRESULT Hr)
{
	if (FAILED(Hr))
	{
		LOG(Error, "ThrowIfFailed: %d", Hr);
		throw std::exception();
	}
}

#define CHECK(condition, Message)          \
	if (!(condition))                      \
	{                                      \
		throw std::runtime_error(Message); \
	}

#define ENSURE(condition) \
	if (!(condition))     \
	{                     \
		__debugbreak();   \
	}
