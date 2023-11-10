#pragma once
#include <memory>
#include <string>

namespace gns
{
	struct Mesh;
	class AssetLoader
	{
	public:
		static std::shared_ptr<Mesh> LoadMesh(std::string path);
	};
}
