#pragma once
#include "../../Materials/Material.h"
#include "../ConfigReader.h"

#include <ranges>
class OMaterialsConfigParser : public OConfigReader
{
public:
	OMaterialsConfigParser(const string& ConfigPath)
	    : OConfigReader(ConfigPath) {}

	std::unordered_map<string, unique_ptr<SMaterial>> LoadMaterials();

	void AddDataToNode(const SMaterial* Mat, boost::property_tree::ptree& OutNode);
	/*Adds or modifies the material in the tree*/
	void AddMaterial(const SMaterial* Material);
	void AddMaterials(const std::unordered_map<string, SMaterial*>& Materials);
	static vector<STexturePath> GetTexturePaths(const boost::property_tree::ptree& Node, const string& Key);
};