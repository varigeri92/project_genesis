#pragma once
#include <string>
#include <unordered_map>
#include "../Utils/Guid.h"

#include "api.h"

namespace gns
{
	namespace rendering
	{
		struct Material;
	}

	enum class AssetType
	{
		mesh,
		texture,
		material,
		shader,
		//--
		graph_object,
		system_graph,
		//--
		audioClip,
		prefab,
		null_Asset
	};

	enum class AssetState
	{
		unloaded, loaded, ready/*, streamed*/
	};


	struct AssetMetadata
	{
		gns::core::guid guid;
		std::string sourcePath;
		std::string name;
		AssetType assetType;
		AssetState state;
	};

	class AssetDatabase
	{
		friend class Engine;
		
	public:
		static GNS_API AssetMetadata& GetAssetByGuid(const gns::core::guid guid);
		static GNS_API AssetMetadata& GetAssetByName(const std::string& assetName);
		static GNS_API bool IsAssetImported(const std::string& assetName, gns::core::guid& guid);
		static GNS_API void SetProjectRoot(const std::string& path);
		static GNS_API void SetResourcesDir(const std::string& path);
		static GNS_API AssetMetadata& AddImportedAssetToDatabase(const AssetMetadata& assetMeta, 
			bool loaded = false, bool isVirtual = false);
		static GNS_API std::string GetExtensionByType(const AssetType type);
	private:
		static void RegisterToMap(const AssetMetadata& assetMeta, bool loaded = false);
		static std::unordered_map<gns::core::guid, AssetMetadata> S_AssetDatabase;
		static void CreateDatabase();
		static void RegisterToFile(gns::core::guid guid);
		static void LoadAssetDatabase();
	};
}

