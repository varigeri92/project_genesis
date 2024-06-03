#include <gnspch.h>
#include "AssetDatabase.h"
#include "AssetLoader.h"
#include "Log.h"

std::unordered_map<gns::core::guid, gns::AssetMetadata> gns::AssetDatabase::S_AssetDatabase = {};

gns::AssetMetadata& gns::AssetDatabase::GetAssetByGuid(const gns::core::guid guid)
{
	LOG_INFO("Getting asset from database: " << guid);
	return S_AssetDatabase[guid];
}

gns::AssetMetadata& gns::AssetDatabase::GetAssetByName(const std::string& assetName)
{
	for (auto it = S_AssetDatabase.begin(); it != S_AssetDatabase.end(); ++it) {
		if (it->second.name == assetName)
			return it->second;
	}
}

bool gns::AssetDatabase::IsAssetImported(const std::string& assetName, gns::core::guid& guid)
{
	for (auto it = S_AssetDatabase.begin(); it != S_AssetDatabase.end(); ++it) {
		if (it->second.name == assetName)
		{
			guid = it->first;
			return true;
		}
	}
	return false;
}

void gns::AssetDatabase::SetProjectRoot(const std::string& path)
{
	AssetLoader::AssetsPath = path + R"(\Assets\)";
}

void gns::AssetDatabase::SetResourcesDir(const std::string& path)
{
	AssetLoader::ResourcesPath = path;
	AssetLoader::ShadersPath = path + R"(\Shaders\)";
}

gns::AssetMetadata& gns::AssetDatabase::AddAssetToDatabase(const AssetMetadata& assetMeta, bool loaded)
{
	if(AssetDatabase::S_AssetDatabase.contains(assetMeta.guid))
	{
		return S_AssetDatabase[assetMeta.guid];
	}

	if(loaded)
	{
		S_AssetDatabase[assetMeta.guid] = {
		assetMeta.guid,
		assetMeta.sourcePath,
		assetMeta.name,
		assetMeta.assetType,
		AssetState::loaded
		};
		return S_AssetDatabase[assetMeta.guid];
	}

	S_AssetDatabase[assetMeta.guid] = {
		assetMeta.guid,
		assetMeta.sourcePath,
		assetMeta.name,
		assetMeta.assetType,
		AssetState::unloaded
	};
	return S_AssetDatabase[assetMeta.guid];
}


