#pragma once
#include <string>
#include <memory>
#include "EnTT/entt.hpp"
namespace gns::core
{
	struct Scene
	{
		std::string name;
		entt::registry registry;
		Scene(std::string name) : name{ name } {};
	};

	class SceneManager
	{
	public:
		SceneManager() = default;
		~SceneManager() = default;
		std::shared_ptr<Scene> CreateScene(std::string name);
		Scene GetScene();
	private:
		std::vector<std::shared_ptr<Scene>> m_scenes;
	};
}

