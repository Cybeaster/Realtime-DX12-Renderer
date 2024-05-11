#pragma once

#include "boost/signals2.hpp"
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"

#include <algorithm> // For std::min and std::max.
#include <cassert>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

struct ORenderItem;
using boost::make_shared;
using boost::optional;
using boost::shared_ptr;
using boost::unordered_map;
using boost::weak_ptr;
using std::array;
using std::make_unique;
using std::map;
using std::pair;
using std::queue;
using std::string;
using std::to_wstring;
using std::unique_ptr;
using std::unordered_set;
using std::vector;
using std::wstring;
#include <algorithm>
#include <string>
#include <unordered_map>

inline bool operator==(const weak_ptr<ORenderItem>& Lhs, const weak_ptr<ORenderItem>& Rhs)
{
	return Lhs.lock() == Rhs.lock();
}

template<typename EnumType>
struct SLitEnum
{
	SLitEnum(EnumType Value, const std::string& Name)
	    : Value(Value), Name(Name) {}

	EnumType Value;
	std::string Name;
};

#define ENUM(Name, ...) \
	enum class Name     \
	{                   \
		__VA_ARGS__     \
	};

#define DEFINE_LIT_ENUM(EnumType, ...)                                                              \
	static const std::unordered_map<EnumType, std::string> EnumType##ToStringMap = { __VA_ARGS__ }; \
	static const std::unordered_map<std::string, EnumType> StringTo##EnumType##Map = []() {         \
		std::unordered_map<std::string, EnumType> map;                                              \
		for (const auto& lit : EnumType##ToStringMap)                                               \
		{                                                                                           \
			map[lit.second] = lit.first;                                                            \
		}                                                                                           \
		return map;                                                                                 \
	}();                                                                                            \
	static const std::string& EnumType##ToString(EnumType Value)                                    \
	{                                                                                               \
		static const std::string unknown = "Unknown";                                               \
		auto it = EnumType##ToStringMap.find(Value);                                                \
		if (it != EnumType##ToStringMap.end())                                                      \
			return it->second;                                                                      \
		return unknown;                                                                             \
	}                                                                                               \
	static EnumType StringTo##EnumType(const std::string& Name)                                     \
	{                                                                                               \
		auto it = StringTo##EnumType##Map.find(Name);                                               \
		if (it != StringTo##EnumType##Map.end())                                                    \
			return it->second;                                                                      \
		return static_cast<EnumType>(-1);                                                           \
	}
