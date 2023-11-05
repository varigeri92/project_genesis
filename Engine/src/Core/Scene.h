#pragma once
#include <string>

namespace gns::core
{
	struct Scene
	{
		std::string name;
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

