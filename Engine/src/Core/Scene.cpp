#include "Scene.h"
#include "Components.h"
#include "../ECS/Entity.h"

namespace gns {

	std::shared_ptr<Scene> SceneManager::CreateScene(std::string name)
	{
		std::shared_ptr<Scene> scene = std::make_shared<Scene>(name);
		m_scenes.push_back(scene);
		return scene;
	}


	Entity Scene::CreateEntity(std::string name, Scene* scene)
	{
		entt::entity entity = registry.create();
		registry.emplace<EntityComponent>(entity, name);
		registry.emplace<Transform>(entity);
		return Entity(entity, scene);
	}
	void Scene::Destroy(Entity& entity)
	{
		registry.destroy(entity.entity);
		delete(&entity);
	}
}
