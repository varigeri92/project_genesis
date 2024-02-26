#pragma once
#include <memory>
#include <string>
#include <vector>
#include "core.h"
#include "gnsAPI.h"

namespace gns::rendering
{
	struct Material;
	struct Mesh;
	class Texture;
}
namespace gns
{
	class AssetLoader
	{
		inline static std::string AssetsPath = "";
		inline static std::string ShadersPath = "";

		static void ProcessImpoertedScene(const void* scene, std::shared_ptr<gns::rendering::Mesh>& mesh);

	public:
		static GNS_API void SetPaths(std::string assetsPath);

		static GNS_API std::vector<uint32_t> LoadShader(std::string path);
		static GNS_API void LoadTextureData(std::string path, rendering::Texture* texture, bool isFallbackPath = false);
		static GNS_API std::vector<std::shared_ptr<gns::rendering::Mesh>> LoadMeshFile(std::string path, bool isFallbackPath = false);
		static GNS_API std::string LoadSceneFromFile();
		static GNS_API std::string GetAssetsPath() { return AssetsPath; }
	};
}
