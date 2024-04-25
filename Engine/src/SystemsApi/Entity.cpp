#include "gnspch.h"
#include "Entity.h"
#include "ComponentLibrary.h"

gns::Entity gns::Entity::CreateEntity(std::string entityName, uint32_t registry)
{
	entt::entity ent = SystemsAPI::GetRegistry(registry).create();
	SystemsAPI::GetRegistry(registry).emplace<EntityComponent>(ent, entityName);
	SystemsAPI::GetRegistry(registry).emplace<Transform>(ent);
	return Entity(ent);

}
