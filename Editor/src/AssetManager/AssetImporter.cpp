#include "AssetImporter.h"
#include <filesystem>
#include <fstream>

#include "../../../Engine/src/AssetDatabase/AssetDatabase.h"
#include "../Utils/Utilities.h"
#define YAML_CPP_STATIC_DEFINE
#include "../../../Engine/src/AssetDatabase/AssetLoader.h"
#include "yaml-cpp/yaml.h"

namespace gns::editor
{
	const std::string metaFileExtension = ".gnsmeta";
	static std::unordered_map<std::string, AssetType> AssetTypeMap = {
	{"obj", AssetType::mesh},
	{"fbx", AssetType::mesh},
	{"gltf", AssetType::mesh},
	{"glb", AssetType::mesh},

	{"png", AssetType::texture},
	{"jpeg", AssetType::texture},
	{"jpg", AssetType::texture}
	};

	bool AssetImporter::ImportAsset(const std::string& assetPath)
	{
		std::string relpath = utils::getRelativePath(AssetLoader::GetAssetsPath(), assetPath);
		LOG_INFO(relpath);
		std::ifstream file(assetPath);
		if (!file.good()) return false;

		if(IsImported(assetPath)) return true;
		
		std::string metaPath = utils::appendExtension(assetPath, metaFileExtension);
		core::guid assetGuid = core::Guid::GetNewGuid();
		AssetType assetType = AssetTypeMap.at(utils::getFileExtension(assetPath));

		CreateMeta(metaPath, relpath, assetGuid, assetType);

		return true;
	}

	bool AssetImporter::ImportAsset(const std::string& assetPath, AssetMetadata& out_assetMeta)
	{
		std::ifstream file(assetPath);
		if (!file.good()) return false;

		std::string relpath = utils::getRelativePath(AssetLoader::GetAssetsPath(), assetPath);
		LOG_INFO(relpath);

		std::string metaPath = utils::appendExtension(assetPath, metaFileExtension);
		if (IsImported(assetPath))
		{
			YAML::Node metaNode = YAML::LoadFile(metaPath);
			out_assetMeta.sourcePath = metaNode["source_path"].as<std::string>();
			out_assetMeta.guid = metaNode["asset_guid"].as<size_t>();
			out_assetMeta.name = metaNode["asset_name"].as<std::string>();
			out_assetMeta.assetType = static_cast<AssetType>(metaNode["asset_type"].as<size_t>());

			return true;
		}
		out_assetMeta.guid = core::Guid::GetNewGuid();
		out_assetMeta.assetType = AssetTypeMap.at(utils::getFileExtension(assetPath));
		CreateMeta(metaPath, relpath, out_assetMeta.guid, out_assetMeta.assetType);
		return true;
	}

	bool AssetImporter::IsImported(const std::string& path) const
	{
		std::ifstream file(utils::appendExtension(path, metaFileExtension));
		return file.good();

	}

	void AssetImporter::CreateMeta(const std::string& path, const std::string& assetSourcePath, 
		core::guid guid, AssetType assetType)
	{
		YAML::Emitter out;
		out << YAML::BeginMap
		<< "asset_name" << "asssetName"
		<< "source_path" << assetSourcePath
		<< "asset_guid" << guid
		<< "asset_type" << static_cast<int>(assetType)
		<< YAML::EndMap;
		utils::writeFile(out.c_str(), path);
	}
}
