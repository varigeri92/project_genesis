#pragma once
#include <memory>
#include <string>
#include <vector>

namespace gns::rendering
{
	struct Mesh;
}
namespace gns
{
	class AssetLoader
	{
	public:
		static std::shared_ptr<gns::rendering::Mesh> LoadMesh(std::string path);
		static std::vector<uint32_t> LoadShader(std::string path);
	};
}
