#pragma once
#include <memory>
#include <string>
#include <vector>

namespace gns
{
	struct Mesh;
	class AssetLoader
	{
	public:
		static std::shared_ptr<Mesh> LoadMesh(std::string path);
		static std::vector<char> LoadFile(std::string path);
		static void* LoadImage(std::string path);
	};
}
