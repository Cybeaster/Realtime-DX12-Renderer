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
		assert(false);
		return nullptr;
	}
}
using TUUID = boost::uuids::uuid;
inline TUUID GenerateUUID()
{
	const boost::uuids::uuid uuid = boost::uuids::random_generator()();
	return uuid;
}