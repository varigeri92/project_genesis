#include "gnspch.h"
#include "Scene.h"
#include "../SystemsApi/ComponentLibrary.h"
#include "entt/entt.hpp"
#include "../../Engine/src/SystemsApi/Entity.h"

namespace gns
{
	Scene::Scene(std::string name) : name(name)
	{
		entt::entity ent = m_registry.create();
		m_registry.emplace<EntityComponent>(ent, "SceneRoot");
		m_registry.emplace<Transform>(ent);
		m_sceneRoot = { ent };
	}

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
	Entity& Scene::GetSceneRoot()
	{
		return m_sceneRoot;
	}
}
