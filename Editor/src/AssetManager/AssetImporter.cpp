﻿#include "AssetImporter.h"
#include "Engine.h"
#include <filesystem>
#include <fstream>
#define YAML_CPP_STATIC_DEFINE
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
	{"jpg", AssetType::texture},
	{"gnsmat", AssetType::material}

	};

	bool AssetImporter::ImportAsset(const std::string& assetPath)
	{
		std::string relpath = gns::fileSystem::FileSystem::GetRelativePath(assetPath);
		LOG_INFO(relpath);
		std::ifstream file(assetPath);
		if (!file.good()) return false;

		if(IsImported(assetPath)) return true;
		
		std::string metaPath = gns::fileSystem::FileSystem::AppendExtension(assetPath, metaFileExtension);
		core::guid assetGuid = core::Guid::GetNewGuid();
		AssetType assetType = AssetTypeMap.at(gns::fileSystem::FileSystem::GetFileExtension(assetPath));

		CreateMeta(metaPath, relpath, assetGuid, assetType);

		return true;
	}

	bool AssetImporter::ImportAsset(const std::string& assetPath, AssetMetadata& out_assetMeta)
	{
		std::ifstream file(assetPath);
		if (!file.good()) return false;
		file.close();

		std::string relpath = gns::fileSystem::FileSystem::GetRelativePath(assetPath);
		LOG_INFO(relpath);
		std::string metaPath = "";
		if(gns::fileSystem::FileSystem::HasExtension(assetPath, metaFileExtension))
		{
			metaPath = assetPath;
		}
		else
		{
			metaPath = gns::fileSystem::FileSystem::AppendExtension(assetPath, metaFileExtension);
		}
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
		out_assetMeta.assetType = AssetTypeMap.at(gns::fileSystem::FileSystem::GetFileExtension(assetPath));
		out_assetMeta.sourcePath = relpath;
		out_assetMeta.name = gns::fileSystem::FileSystem::GetFileName(relpath);
		CreateMeta(metaPath, relpath, out_assetMeta.guid, out_assetMeta.assetType);
		return true;
	}

	bool AssetImporter::IsImported(const std::string& path) const
	{
		if(gns::fileSystem::FileSystem::HasExtension(path, metaFileExtension))
			return true;
		
		std::ifstream file(gns::fileSystem::FileSystem::AppendExtension(path, metaFileExtension));
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
		gns::fileSystem::FileSystem::WriteFile(out.c_str(), path);
	}

	AssetMetadata AssetImporter::CreateAsset(const std::string& directory, const std::string& name, core::guid guid,
	                                          AssetType assetType)
	{
		std::string absPath = directory + name + AssetDatabase::GetExtensionByType(assetType);
		std::string relPath = gns::fileSystem::FileSystem::GetRelativePath(absPath);
		YAML::Emitter out;
		out << YAML::BeginMap
			<< "asset_name" << name
			<< "source_path" << relPath
			<< "asset_guid" << guid
			<< "asset_type" << static_cast<int>(assetType)
			<< YAML::EndMap;
		gns::fileSystem::FileSystem::WriteFile(out.c_str(), absPath);

		AssetMetadata assetMetadata;
		if (ImportAsset(absPath, assetMetadata))
		{
			return assetMetadata;
		}
		return assetMetadata;
	}
}
