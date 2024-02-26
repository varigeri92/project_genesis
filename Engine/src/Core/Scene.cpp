#include "Scene.h"
#include "Components.h"
#include "../ECS/Entity.h"

namespace gns {
	Entity Scene::CreateEntity(std::string name, Scene* scene)
	{
		entt::entity e = registry.create();
		Entity entity(e, scene);
		entity.AddComponet<EntityComponent>(name);
		entity.AddComponet<Transform>();
		return entity;
	}
	void Scene::Destroy(Entity& entity)
	{
		registry.destroy(entity.entity);
		delete(&entity);
	}

	void Scene::Serialize()
	{
		SerializeRegistry();
	}

	void Scene::SerializeRegistry()
	{
		LOG_INFO("WIP");
	}
}
