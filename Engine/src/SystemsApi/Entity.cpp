#include "gnspch.h"
#include "Entity.h"
#include "ComponentLibrary.h"
#include "../Level/SceneManager.h"
#include "../Utils/Serialization/Serializer.h"

gns::Entity gns::Entity::CreateEntity(std::string entityName, core::Scene* scene)
{
	entt::entity ent = SystemsAPI::GetRegistry().create();
	SystemsAPI::GetRegistry().emplace<EntityComponent>(ent, entityName);
	SystemsAPI::GetRegistry().emplace<Transform>(ent);
    scene->AddEntity(ent);
    Entity entity(ent);
	entity.componentsVector = {};
	return entity;

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
