#pragma once
#include <string>
#include <memory>
#include "EnTT/entt.hpp"

namespace gns
{
	class Entity;
	struct Scene
	{
		std::string name;
		entt::registry registry;
		Scene(std::string name) : name{ name } {};

		Entity CreateEntity(std::string name, Scene* scene);
		void Destroy(Entity& entity);
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

