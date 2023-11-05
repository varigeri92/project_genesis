#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace gns::core
{
	typedef uint64_t Entity;
	struct Scene
	{
		std::string name;
		std::vector<Entity> _entities;
	};

	class SceneManager
	{
	public:
		SceneManager() = default;
		~SceneManager() = default;
		Scene CreateScene();
		Scene LoadScene();
		Scene GetScene();
	private:
	};
}

