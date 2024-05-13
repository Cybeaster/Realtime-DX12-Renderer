#pragma once
#include "Logger.h"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"

class OConfigReader
{
public:
	OConfigReader(const string& ConfigPath)
	    : FileName(ConfigPath)
	{
		LoadConfig(FileName);
	}

	void LoadConfig(const string& FileName)
	{
		if (!bIsLoaded)
		{
			read_json(FileName, PTree);
			CWIN_LOG(PTree.size() == 0, Default, Error, "Config file is empty!")
			bIsLoaded = true;
		}
	}

	void ReloadConfig()
	{
		PTree.clear();
		bIsLoaded = false;
		LoadConfig(FileName);
	}

	static auto GetAttribute(const boost::property_tree::ptree& Tree, const string& Value)
	{
		return Tree.get<string>(Value);
	}

	template<typename T>
	T GetRoot(const std::string& Key) const
	{
		CWIN_LOG(!bIsLoaded, Default, Error, "Config file not loaded!");
		if (PTree.get_child_optional(Key) == boost::none)
		{
			LOG(Debug, Error, "Key not found: {}", TEXT(Key));
		}
		return PTree.get<T>(Key);
	}

	auto GetRootChild(const std::string& Key) const
	{
		CWIN_LOG(!bIsLoaded, Default, Error, "Config file not loaded!");
		if (PTree.get_child_optional(Key) == boost::none)
		{
			LOG(Debug, Error, "Key not found: {}", TEXT(Key));
		}

		return PTree.get_child(Key);
	}

	auto GetChild(const std::string& Key, const boost::property_tree::ptree& Tree) const
	{
		CWIN_LOG(!bIsLoaded, Default, Error, "Config file not loaded!");
		if (Tree.get_child_optional(Key) == boost::none)
		{
			LOG(Debug, Error, "Key not found: {}", TEXT(Key));
		}

		return Tree.get_child(Key);
	}

	template<typename T>
	static T GetOptionalOr(const boost::property_tree::ptree& Tree, const std::string& Key, const T& Default)
	{
		if (auto Optional = Tree.get_optional<T>(Key))
		{
			return Optional.get();
		}
		return Default;
	}

	template<typename T>
	static T GetOptionalOr(const boost::optional<const boost::property_tree::ptree&>& Tree, const std::string& Key, const T& Default)
	{
		if (Tree)
		{
			return GetOptionalOr<T>(Tree.get(), Key, Default);
		}
		return Default;
	}

	static string GetOptionalOr(const boost::property_tree::ptree& Tree, const std::string& Key, const char* Default)
	{
		return GetOptionalOr<string>(Tree, Key, Default);
	}

	static string GetOptionalOr(const boost::optional<const boost::property_tree::ptree&>& Tree, const std::string& Key, const char* Default)
	{
		if (Tree)
		{
			return GetOptionalOr<string>(Tree.get(), Key, Default);
		}
		return Default;
	}

protected:
	string FileName;
	boost::property_tree::ptree PTree;
	bool bIsLoaded = false;
};
