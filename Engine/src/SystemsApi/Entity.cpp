#include "gnspch.h"
#include "Entity.h"
#include "ComponentLibrary.h"
#include "../Utils/Serialization/Serializer.h"

gns::Entity gns::Entity::CreateEntity(std::string entityName, uint32_t registry)
{
	entt::entity ent = SystemsAPI::GetRegistry(registry).create();
	SystemsAPI::GetRegistry(registry).emplace<EntityComponent>(ent, entityName);
	SystemsAPI::GetRegistry(registry).emplace<Transform>(ent);
	return Entity(ent);

}

std::vector<gns::ComponentMetadata> gns::Entity::GetAllComponent()
{
    std::vector<gns::ComponentMetadata> componentsVector = {};
    for (auto&& curr : SystemsAPI::GetDefaultRegistry().storage())
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
