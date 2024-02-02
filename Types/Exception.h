#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Types.h"
#include "comdef.h"

#include <exception>
#include <string>
struct SDXException
{
public:
	SDXException() = default;
	SDXException(HRESULT Hr, const std::wstring& FuncName, const std::wstring& FileName, int LineNumber)
	    : ErrorCode(Hr), FunctionName(FuncName), FileName(FileName), LineNumber(LineNumber)
	{
	}

	std::wstring ToString() const
	{
		_com_error err(ErrorCode);
		std::wstring msg = err.ErrorMessage();

		return FunctionName + L" failed in " + FileName + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
	}

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring FileName;
	int LineNumber = -1;
};

#ifndef THROW_IF_FAILED
#define THROW_IF_FAILED(hr)                                                           \
	{                                                                                 \
		if (FAILED(hr))                                                               \
		{                                                                             \
			auto __exception = SDXException(hr, L#hr, __FILEW__, __LINE__);           \
			MessageBox(nullptr, __exception.ToString().c_str(), L"HR Failed", MB_OK); \
			throw __exception;                                                        \
		}                                                                             \
	}
#endif

#define CHECK_MSG(condition, Message)      \
	if (!(condition))                      \
	{                                      \
		throw std::runtime_error(Message); \
	}

#define CHECK(condition) CHECK_MSG(condition, "Condition failed")

#define ENSURE(condition) \
	if (!(condition))     \
	{                     \
		__debugbreak();   \
	}
