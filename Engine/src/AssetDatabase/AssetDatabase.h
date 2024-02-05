#pragma once
#include <unordered_map>
#include "Guid.h"

#include "gnsAPI.h"

namespace gns
{
	struct AssetMetadata
	{
		std::string Path;
		std::string AssetType;
	};

	class AssetDatabase
	{
	public:
		static GEN_API void ImportAsset(std::string path);
		static GEN_API AssetMetadata GetAssetByGuid(gns::core::guid guid);
	private:
		static std::unordered_map<gns::core::guid, AssetMetadata> S_AssetDatabase;

	};
}

