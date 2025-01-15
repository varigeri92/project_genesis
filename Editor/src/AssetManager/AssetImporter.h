#pragma once
#include <string>
#include "Engine.h"

namespace gns
{
	enum class AssetType;
}

namespace gns::editor
{

	class AssetImporter
	{
	public:
		AssetImporter() = default;
		~AssetImporter() = default;

		bool ImportAsset(const std::string& assetPath);
		bool ImportAsset(const std::string& assetPath, AssetMetadata& out_assetMeta);
		AssetMetadata CreateAsset(
			const std::string& directory, 
			const std::string& name, core::guid guid, AssetType assetType);

		AssetMetadata CreateAssetFromObject(
			const std::string& directory,
			const std::string& name, core::guid guid, AssetType assetType, Object* gns_object);

		static gns::core::guid GetGuidFromMetaFile(const std::string& asset_path);

	private:
		void ScanProjectForAssets();
		bool IsImported(core::guid);
		void MoveAsset(core::guid guid, const std::string& targetPath);

		bool IsImported(const std::string& path) const;
		void CreateMeta(const std::string& path, const std::string& assetSourcePath, core::guid guid, AssetType assetType);
		YAML::Emitter& CreateAssetBaseData(YAML::Emitter& out, const std::string& rel_path, const std::string& name, core::guid guid,
			AssetType assetType);

		YAML::Emitter& SerializeMaterialValues(YAML::Emitter& out, gns::rendering::Material* material, const std::string& attributeName,
			rendering::Shader::ShaderAttributeType attributeType_id);
	};
}
