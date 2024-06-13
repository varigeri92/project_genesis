#include "gnspch.h"
#include "Scene.h"
#include "../SystemsApi/ComponentLibrary.h"
namespace gns
{
	Scene::Scene(std::string name) : name(name) {}

	Entity Scene::SoftCreateEntity()
	{
		entt::entity ent = m_registry.create();
		Entity entity(ent);
		entity.componentsVector = {};
		return entity;
	}

	Entity Scene::CreateEntity(std::string name)
	{
		return Entity::CreateEntity(name, this);
	}

	Entity Scene::FindEntity(std::string name)
	{
		auto view = m_registry.view<EntityComponent>();
		for (auto [entt, entity] : view.each())
		{
			if (entity.name == name)
			{
				return Entity{ entt };
			}
		}
		return {static_cast<entt::entity>(entt::null)};
	}

	Entity Scene::FindEntity(gns::core::guid guid)
	{
		auto view = m_registry.view<EntityComponent>();
		for (auto [entt, entity] : view.each())
		{
			if(entity.guid == guid)
			{
				return Entity{ entt };
			}
		}
		return {static_cast<entt::entity>(entt::null)};
	}

	std::vector<Entity> Scene::GetEntities(gns::core::guid name)
	{
		return {};
	}

	entt::registry& Scene::Registry()
	{
		return m_registry;
	}
}
