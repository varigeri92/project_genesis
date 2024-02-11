#include "SystemsApi.h"

#include "Log.h"

std::vector<gns::core::SystemBase*> gns::core::SystemsApi::Systems = {};

void gns::core::SystemsApi::UpdateSystems()
{
	for (auto system : Systems)
	{
		if(system->isActive)
			system->OnUpdate();
	}
}
