#include "gnspch.h"
#include "Entity.h"
#include "ComponentLibrary.h"
#include "../Level/SceneManager.h"
#include "../Utils/Serialization/Serializer.h"

gns::Entity gns::Entity::CreateEntity(std::string entityName, Scene* scene)
{
	entt::entity ent = scene->m_registry.create();
    scene->m_registry.emplace<EntityComponent>(ent, entityName);
    scene->m_registry.emplace<Transform>(ent);
    Entity entity(ent);
	entity.componentsVector = {};
	return entity;
}

void gns::Entity::RemoveChild(entt::entity ent)
{
    if (!IsValid())
        return;

    Entity(ent).SetParent(entt::null);
}
void gns::Entity::SetParent(entt::entity parent)
{
    Entity e = { parent };

	if (parent == entity)
	{
        LOG_INFO("Cannot set 'self' as Parent...");
        return;
	}
    SetParent(e);
}

gns::Entity gns::Entity::GetParent()
{
    return { GetComponent<Transform>().parent };
}

std::vector<gns::Entity>& gns::Entity::GetChildren()
{
    children.clear();
    Transform& transform = GetComponent<Transform>();
    for (const auto e : transform.children)
    {
        children.emplace_back(e);
    }
	return children;
}

void gns::Entity::SetParent(Entity& parent)
{
    GetParent().RemoveChild(entity);
    if(!parent.IsValid())
        return;

    parent.GetComponent<Transform>().children.emplace_back(entity);
    parent.GetComponent<Transform>().children_guid.emplace_back(GetComponent<EntityComponent>().guid);
}


void gns::Entity::Delete()
{
    GetParent().RemoveChild(entity);
    SystemsAPI::GetRegistry().destroy(entity);
}

const std::vector<gns::ComponentMetadata>& gns::Entity::GetAllComponent()
{
    componentsVector.clear();
    for (auto&& curr : SystemsAPI::GetRegistry().storage())
    {
        entt::id_type id = curr.first;

        if (auto& storage = curr.second; storage.contains(entity))
        {
            if(Serializer::ComponentData_Table.contains(id))
            {
                void* component_ptr = storage.value(entity);
            	componentsVector.emplace_back(component_ptr, id);
            }
        }
    }
	return componentsVector;
}
