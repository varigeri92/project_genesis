#include "gnspch.h"
#include "SystemsAPI.h"

#include "ComponentLibrary.h"
#include "../Level/SceneManager.h"


std::vector<gns::SystemBase*> gns::SystemsAPI::Systems = {};

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
	for (const auto system : Systems)
	{
		delete system;
	}
}

entt::registry& gns::SystemsAPI::GetRegistry()
{
	return core::SceneManager::ActiveScene->Registry();
}
