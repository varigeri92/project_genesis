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
	private:
		void ScanProjectForAssets();
		bool IsImported(core::guid);
		void MoveAsset(core::guid guid, const std::string& targetPath);

		bool IsImported(const std::string& path) const;
		void CreateMeta(const std::string& path, const std::string& assetSourcePath, core::guid guid, AssetType assetType);
		std::string GetExtensionByType(const AssetType type) const;
	};
}
