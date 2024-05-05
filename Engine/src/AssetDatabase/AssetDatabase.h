#pragma once
#include <string>
#include <unordered_map>
#include "../Utils/Guid.h"

#include "api.h"

namespace gns
{
	enum class AssetType
	{
		mesh,
		texture,
		audioClip,
		prefab
	};
	struct AssetMetadata
	{
		std::string path;
		AssetType assetType;
	};

	class AssetDatabase
	{
	public:
		static GNS_API void ImportAsset(std::string path);
		static GNS_API AssetMetadata GetAssetByGuid(const gns::core::guid guid);
		static GNS_API void SetProjectRoot(const std::string& path);
		static GNS_API void SetResourcesDir(const std::string& path);
	private:
		static std::unordered_map<gns::core::guid, AssetMetadata> S_AssetDatabase;

	};
}

