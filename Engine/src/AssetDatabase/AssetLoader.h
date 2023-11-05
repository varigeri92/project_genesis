#pragma once
#include <memory>
#include "../Rendering/Renderer.h"
namespace gns
{
	struct Mesh;
	class AssetLoader
	{
	public:
		static std::shared_ptr<Mesh> LoadMesh(std::string path);
	};
}
