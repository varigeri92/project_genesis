#include "Scene.h"
namespace gns {

	std::shared_ptr<Scene> SceneManager::CreateScene(std::string name)
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>(name);
		m_scenes.push_back(scene);
		return scene;
	}
}
