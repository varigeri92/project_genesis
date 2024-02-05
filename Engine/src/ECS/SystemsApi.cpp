#include "SystemsApi.h"

#include "Log.h"

std::vector<gns::core::SystemBase*> gns::core::SystemsApi::Systems = {};

void gns::core::SystemsApi::RegisterSystem(gns::core::SystemBase* system)
{
	if(std::find(Systems.begin(), Systems.end(), system) != Systems.end())
	{
		LOG_WARNING("System With the Same type has already been Created!");
		return;
	}
	gns::core::SystemsApi::Systems.push_back(system);
	system->index = Systems.size()-1;
	system->OnCreate();
}

void gns::core::SystemsApi::UnRegisterSystem(gns::core::SystemBase* system)
{
	auto sys = std::find(Systems.begin(), Systems.end(), system);
	if(sys == Systems.end())
	{
		LOG_WARNING("SOMETHING IS REALY BAD!!");
		return;
	}
	system->OnDestroy();
	Systems.erase(sys);
}

void gns::core::SystemsApi::UpdateSystems()
{
	for (auto system : Systems)
	{
		system->OnUpdate();
	}
}
