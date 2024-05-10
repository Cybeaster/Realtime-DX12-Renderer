#pragma once
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

template<typename T>
T* Cast(auto* Ptr)
{
	auto casted = dynamic_cast<T*>(Ptr);
	if (casted != nullptr)
	{
		return casted;
	}
	else
	{
		return nullptr;
	}
}

template<typename T>
T* SCast(auto* Ptr)
{
	return static_cast<T*>(Ptr);
}

template<typename T>
T SCast(auto Ptr)
{
	return static_cast<T>(Ptr);
}

inline void* PtrCast(uint64_t Ptr)
{
	return reinterpret_cast<void*>(Ptr);
}

using TUUID = string;
inline TUUID GenerateUUID()
{
	// Generate a new UUID
	const boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// Convert UUID to string
	return to_string(uuid);
}