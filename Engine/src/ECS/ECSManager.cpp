#include "ECSManager.h"
#include "../Core/Components.h"
#include <glm/gtc/matrix_transform.hpp>

namespace gns
{
	Entity ECSManager::CreateEntity(std::string name, std::shared_ptr<Scene> scene)
	{
		entt::entity entity = scene->registry.create();
		scene->registry.emplace<EntityComponent>(entity, name);
		scene->registry.emplace<Transform>(entity);
		return Entity(entity, scene);
	}
	Entity ECSManager::CreateEntity(std::shared_ptr<Scene> scene)
	{
		return CreateEntity("new entity", scene);
	}
	void ECSManager::Destroy(Entity& entity, std::shared_ptr<Scene> scene)
	{
		scene->registry.destroy(entity.entity);
		delete(&entity);
	}
}