#pragma once
#include "SystemBase.h"

namespace gns::core
{
	
class SystemsApi
{
private:
	static std::vector<gns::core::SystemBase*> Systems;

public:
	static void GEN_API RegisterSystem(gns::core::SystemBase* system);
	static void GEN_API UnRegisterSystem(gns::core::SystemBase* system);
	static void GEN_API UpdateSystems();
};

}

