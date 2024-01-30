#pragma once
#include <memory>
#include <string>
#include <vector>

#define ASSETS_PATH "D:\\GenesisEngine\\Engine\\Assets\\"
namespace gns::rendering
{
	struct Mesh;
	class Texture;
}
namespace gns
{
	class AssetLoader
	{
		inline static std::string AssetsPath = ASSETS_PATH;
	public:
		static std::shared_ptr<gns::rendering::Mesh> LoadMesh(std::string path);
		static std::shared_ptr<gns::rendering::Mesh> LoadMeshIndexed(std::string path);
		static std::vector<uint32_t> LoadShader(std::string path);
		static void LoadTextureData(std::string path, rendering::Texture* texture);
	};
}
