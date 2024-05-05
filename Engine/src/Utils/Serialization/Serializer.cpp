#include "gnspch.h"
#include "Serializer.h"
#include "yaml-cpp/yaml.h"

std::unordered_map<uint32_t, ComponentData> Serializer::ComponentData_Table = {};

void Serializer::SerializeToFile(void* src, std::string out_path)
{
	
}

void Serializer::SerializeScene(gns::core::Scene* scene)
{
}

void Serializer::RegisterSerializableComponents(const std::function<void()>& callback)
{
	callback();
}
