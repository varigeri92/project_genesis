#pragma once
#include <unordered_map>
#include "Guid.h"
#include <yaml-cpp/yaml.h>

struct AssetMetadata
{
	std::string Path;
	std::string AssetType;
};

class AssetDatabase
{
public:
	static void ImportAsset(std::string path);
	static AssetMetadata GetAssetByGuid(gns::core::guid guid);
private:
	static std::unordered_map<gns::core::guid, AssetMetadata> S_AssetDatabase;

};

