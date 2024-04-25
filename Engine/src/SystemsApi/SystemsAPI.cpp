#include "gnspch.h"
#include "SystemsAPI.h"

#include "ComponentLibrary.h"


std::vector<gns::SystemBase*> gns::SystemsAPI::Systems = {};
entt::registry gns::SystemsAPI::m_registry = {};

void gns::SystemsAPI::UpdateSystems(float deltaTime)
{
	for (const auto system : Systems)
	{
		system->OnUpdate(deltaTime);
	}
}

void gns::SystemsAPI::ClearSystems()
{
	LOG_INFO(GREEN << "[SYSTEMS API]: "<< DEFAULT <<"Clearing registry!");
	m_registry.view<EntityComponent>().each([&](const auto entity, auto &&...) {
		m_registry.destroy(entity);
		});
	for (const auto system : Systems)
	{
		delete system;
	}

}

entt::registry& gns::SystemsAPI::GetDefaultRegistry()
{
	return m_registry;
}

entt::registry& gns::SystemsAPI::GetRegistry(uint32_t registryIndex)
{
	return m_registry;
}
