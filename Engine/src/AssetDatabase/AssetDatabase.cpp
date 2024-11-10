#include <gnspch.h>
#include "AssetDatabase.h"
#include "AssetLoader.h"
#include "Log.h"
#include "../../../Editor/src/AssetManager/AssetImporter.h"
#include "../Utils/FileSystem/FileSystem.h"
#include "yaml-cpp/yaml.h"

std::unordered_map<gns::core::guid, gns::AssetMetadata> gns::AssetDatabase::S_AssetDatabase = {
{0,{0, "" ,"missing_asset", AssetType::null_Asset, AssetState::unloaded} }
};

gns::AssetMetadata& gns::AssetDatabase::GetAssetByGuid(const gns::core::guid guid)
{
	LOG_INFO("Getting asset from database: " << guid << " '" << S_AssetDatabase[guid].name << "'");

	if(!S_AssetDatabase.contains(guid) || guid == 0)
	{
		LOG_WARNING("[Asset Database]: No asset found with Guid: '"<< guid <<"'");
		return S_AssetDatabase[0];
	}

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
	AssetLoader::ProjectPath = path + R"(\)";
	AssetLoader::AssetsPath = path + R"(\Assets\)";
}

void gns::AssetDatabase::SetResourcesDir(const std::string& path)
{
	AssetLoader::ResourcesPath = path;
	AssetLoader::ShadersPath = path + R"(\Shaders\)";
}

gns::AssetMetadata& gns::AssetDatabase::AddImportedAssetToDatabase(const AssetMetadata& assetMeta, bool loaded, bool isVirtual)
{
	RegisterToMap(assetMeta, loaded);
	if(!isVirtual)
		RegisterToFile(assetMeta.guid);
	return S_AssetDatabase[assetMeta.guid];
}

std::string gns::AssetDatabase::GetExtensionByType(const AssetType type)
{
	switch (type)
	{
	case AssetType::material:
		return ".gnsmat";
	case AssetType::prefab:
		return ".gnsprefab";
	default:
		return ".UNDEFINED";
	}
}

void gns::AssetDatabase::RegisterToMap(const AssetMetadata& assetMeta, bool loaded)
{
	if (AssetDatabase::S_AssetDatabase.contains(assetMeta.guid))
	{
		return;
	}

	if (loaded)
	{
		S_AssetDatabase[assetMeta.guid] = {
		assetMeta.guid,
		assetMeta.sourcePath,
		assetMeta.name,
		assetMeta.assetType,
		AssetState::loaded
		};
		return;
	}

	S_AssetDatabase[assetMeta.guid] = {
		assetMeta.guid,
		assetMeta.sourcePath,
		assetMeta.name,
		assetMeta.assetType,
		AssetState::unloaded
	};
}

void gns::AssetDatabase::CreateDatabase()
{
	for (auto it = S_AssetDatabase.begin(); it != S_AssetDatabase.end(); ++it) 
	{
		RegisterToFile(it->first);
	}
}

void gns::AssetDatabase::RegisterToFile(gns::core::guid guid)
{
	YAML::Emitter out;
	out << YAML::BeginMap
		<< "asset_guid" << guid
		<< "asset_source" << S_AssetDatabase[guid].sourcePath
		<< "asset_meta" << gns::fileSystem::FileSystem::AppendExtension(S_AssetDatabase[guid].sourcePath,".gnsmeta");
	gns::fileSystem::FileSystem::WriteFile(out.c_str(), 
		AssetLoader::GetProjectPath() + ".assetdatabase\\" + std::to_string(guid));
}

void gns::AssetDatabase::LoadAssetDatabase()
{
	LOG_INFO("Loading database!");
	std::vector<std::string> files = gns::fileSystem::FileSystem::GetAllFileInDirectory(
		AssetLoader::GetProjectPath() + ".assetdatabase\\");
	for (auto file : files)
	{
		YAML::Node root = YAML::LoadFile(file);
		core::guid guid = root["asset_guid"].as<size_t>();
		std::string src_file = AssetLoader::GetAssetsPath() + root["asset_source"].as<std::string>();
		if(gns::fileSystem::FileSystem::Exists(src_file))
		{
			std::string meta_file = AssetLoader::GetAssetsPath() + root["asset_meta"].as<std::string>();
			if (gns::fileSystem::FileSystem::Exists(meta_file))
			{
				YAML::Node metaNode = YAML::LoadFile(meta_file);
				S_AssetDatabase[guid] = {
					metaNode["asset_guid"].as<size_t>(),
					metaNode["source_path"].as<std::string>(),
					metaNode["asset_name"].as<std::string>(),
					static_cast<AssetType>(metaNode["asset_type"].as<size_t>()),
					AssetState::unloaded
				};
			}	
		}
	}

	for (auto it = S_AssetDatabase.begin(); it != S_AssetDatabase.end(); ++it)
	{
		LOG_INFO("Asset Registered From database:'" << it->first << "' -> " << it->second.name);
	}
}


