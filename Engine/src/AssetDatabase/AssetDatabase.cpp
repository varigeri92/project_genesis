#include <gnspch.h>
#include "AssetDatabase.h"
#include "AssetLoader.h"
#include "Log.h"

std::unordered_map<gns::core::guid, gns::AssetMetadata> gns::AssetDatabase::S_AssetDatabase = {};
void gns::AssetDatabase::ImportAsset(std::string path)
{
	LOG_INFO("IMPORTING ASSET... TBD");
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
