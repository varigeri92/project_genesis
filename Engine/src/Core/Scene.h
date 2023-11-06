#pragma once
#include <string>
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
		Scene CreateScene();
		Scene LoadScene();
		Scene GetScene();
		void Update();
	private:
	};
}

