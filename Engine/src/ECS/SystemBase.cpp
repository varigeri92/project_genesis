#include "SystemBase.h"
#include "SystemsApi.h"

gns::core::SystemBase::SystemBase(std::string name) :name(name)
{
	guid = gns::core::Guid::GetNewGuid();

	//SystemsApi::RegisterSystem(this);
}
