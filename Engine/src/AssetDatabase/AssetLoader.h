#pragma once
#include <memory>
#include <string>
#include <vector>

#include "gnsAPI.h"

#define ASSETS_PATH "D:\\GenesisEngine\\Engine\\Assets\\"
#define SHADERS_PATH "D:\\GenesisEngine\\Engine\\Assets\\Shaders\\"
namespace gns::rendering
{
	struct Mesh;
	class Texture;
}
namespace gns
{
	class AssetLoader
	{
		inline static std::string AssetsPath = "";
		inline static std::string ShadersPath = SHADERS_PATH;
	public:
		static GEN_API void SetPaths(std::string assetsPath);
		static GEN_API std::shared_ptr<gns::rendering::Mesh> LoadMesh(std::string path);
		static GEN_API std::shared_ptr<gns::rendering::Mesh> LoadMeshIndexed(std::string path);
		static GEN_API std::vector<uint32_t> LoadShader(std::string path);
		static GEN_API void LoadTextureData(std::string path, rendering::Texture* texture);
	};
}
