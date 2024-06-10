#pragma once
#include <memory>
#include <string>
#include <vector>
#include "api.h"
#include "AssetDatabase.h"
#include "../Utils/Guid.h"

namespace gns::rendering
{
	struct Mesh;
	struct Material;
	struct MeshData;
	class Texture;
}
namespace gns
{
	struct AssetMetadata;

	class AssetLoader
	{
		friend class AssetDatabase;
		static std::string AssetsPath;
		static std::string ShadersPath;
		static std::string ResourcesPath;

		static void ProcessImpoertedScene(const void* scene, gns::rendering::MeshData*);

		static GNS_API void* LoadAssetFromFile_internal(AssetMetadata& metaData);
	public:
		static GNS_API std::vector<uint32_t> LoadShader(std::string path);
		static GNS_API void LoadTextureData(std::string path, rendering::Texture* texture, bool isFallbackPath = false);
		static GNS_API void LoadTextureFromResources(std::string path, rendering::Texture* texture);
		static GNS_API rendering::Mesh* LoadMeshFile(gns::core::guid guid, std::string path, bool isFallbackPath = false);
		///<summary> Returns the path to the Assets directory with '\' at the end </summary>
		static GNS_API std::string& GetAssetsPath() { return AssetsPath; }
		static GNS_API std::string& GetEngineResourcesPath() { return ResourcesPath; }

		template<typename T>
		static T* LoadAssetFromFile(core::guid guid)
		{
			void* assetContent_ptr = 
				LoadAssetFromFile_internal(AssetDatabase::GetAssetByGuid(guid));
			return static_cast<T*>(assetContent_ptr);
		}
	};
}
