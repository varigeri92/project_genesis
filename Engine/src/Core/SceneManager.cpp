#include "SceneManager.h"

#include "Scene.h"

std::map<gns::core::guid, std::shared_ptr<gns::Scene>> gns::SceneManager::Scenes = {};
gns::core::guid gns::SceneManager::ActiveSceneGuid = 0;

gns::Scene* gns::SceneManager::GetActiveScene()
{
	return Scenes[ActiveSceneGuid].get();
}

gns::Scene* gns::SceneManager::CreateScene(std::string name)
{
	const core::guid scene_guid = gns::core::Guid::GetNewGuid();
	if(ActiveSceneGuid == 0)
		ActiveSceneGuid = scene_guid;

	Scenes.emplace(scene_guid, std::make_shared<Scene>(name));
	return Scenes[scene_guid].get();
}
