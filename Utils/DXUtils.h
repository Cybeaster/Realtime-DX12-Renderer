#pragma once
#include <d3d12.h>
namespace Utils
{
inline auto CalcBufferByteSize(const UINT ByteSize)
{
	return (ByteSize + 255) & ~255;
}
} // namespace Utils