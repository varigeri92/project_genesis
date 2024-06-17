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
    entity.SetParent(core::SceneManager::ActiveScene->GetSceneRoot());
	return entity;
}

void gns::Entity::RemoveChild(entt::entity ent)
{
    LOG_INFO("Remove Children of: " << (uint32_t)entity << " child was:" <<(uint32_t)ent);
    if (!IsValid())
        return;

    gns::core::guid guid = Entity(ent).GetComponent<EntityComponent>().guid;
    size_t index = 0;
    Transform& t = GetComponent<Transform>();
    for (size_t i = 0; i < t.children.size(); i++)
    {
        if (t.children[i] == ent) {
            index = i;
            t.children.erase(t.children.begin()+ index);
            t.children_guid.erase(t.children_guid.begin() + index);
            break;
        }
    }
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

bool gns::Entity::IsValid()
{
    return entity != entt::null;
}

gns::Entity gns::Entity::GetParent()
{
    entt::entity parent = GetComponent<Transform>().parent;
    LOG_INFO("The parent of " << (uint32_t)entity << "is: " << (uint32_t)parent);
    return { parent };
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
    if(!parent.IsValid())
        return;
    Entity oldParent = GetParent();
    if (oldParent.entity == parent.entity) {
        return;
    }
    oldParent.RemoveChild(entity);
    GetComponent<Transform>().parent = parent.entity;
    parent.GetComponent<Transform>().children.emplace_back(entity);
    parent.GetComponent<Transform>().children_guid.emplace_back(GetComponent<EntityComponent>().guid);
    LOG_INFO("Set Parent of: " << (uint32_t)entity << " to " << (uint32_t)parent.entity << ", old parent was: " << (uint32_t)oldParent.entity);

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
