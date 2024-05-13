#pragma once
#include "Serializeable.h"
#include "api.h"

namespace gns::core
{
	struct Scene;
}

class Serializer
{
public:
	GNS_API static std::unordered_map<uint32_t, ComponentData> ComponentData_Table;

	GNS_API void SerializeToFile(void* src, std::string out_path);
	GNS_API void SerializeScene(gns::core::Scene* scene);

	GNS_API static void RegisterSerializableComponents(const std::function<void()>& callback);

};
