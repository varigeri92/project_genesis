#include "gnspch.h"
#include "SceneManager.h"

namespace gns::core
{
	Scene* SceneManager::ActiveScene = nullptr;
	Scene* SceneManager::CreateScene(std::string name)
	{
		Scene* newScene = new Scene();
		if (ActiveScene == nullptr)
			ActiveScene = newScene;

		IntegrateScene(newScene);
		SerializeScene(newScene);
		return newScene;
	}

	Scene* SceneManager::SerializeScene(Scene* scene)
	{
		SystemsAPI::GetDefaultRegistry().each([scene](entt::entity e)
		{
			Entity entity(e);
			LOG_INFO(entity.GetComponent<EntityComponent>().name);
			scene->sceneEntities.emplace_back();
			SceneEntity& sceneEntity = scene->sceneEntities[scene->sceneEntities.size() - 1];
		});
		return scene;
	}

	void SceneManager::IntegrateScene(Scene* scene)
	{
		for (const auto& sceneEntity : scene->sceneEntities)
		{
			entt::entity entt = SystemsAPI::GetRegistry(0).create();
			for (const auto& component : sceneEntity.componentDataArray)
			{

			}
		}
	}
}
