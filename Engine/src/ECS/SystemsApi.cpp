#include "SystemsApi.h"
#include "../Core/Time.h"
#include "Log.h"

std::vector<gns::core::SystemBase*> gns::core::SystemsApi::Systems = {};

void gns::core::SystemsApi::UpdateSystems()
{
	for (auto system : Systems)
	{
		const int64_t st = Time::GetNow();
		if (system->isActive)
		{
			system->OnUpdate();
		}
		const int64_t et = Time::GetNow();
		const int64_t elapsed = et - st;
		system->time = elapsed;
	}
}
