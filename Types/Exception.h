#pragma once
#define WIN32_LEAN_AND_MEAN
#include <exception>
#include <windows.h>

inline void ThrowIfFailed(HRESULT Hr)
{
	if (FAILED(Hr))
	{
		throw std::exception();
	}
}

#define CHECK(condition,Message) \
if (!(condition)) { \
throw std::runtime_error(Message); \
}

#define ENSURE(condition) \
if (!(condition)) { \
__debugbreak(); \
}
