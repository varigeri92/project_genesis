#pragma once
#include "Serializeable.h"
#include "api.h"
#include "../Level/Scene.h"
namespace YAML
{
	class Node;
	class Emitter;
}

class Serializer
{
public:

	GNS_API static std::unordered_map<uint32_t, ComponentData> ComponentData_Table;

	GNS_API static std::unordered_map<size_t, std::function<void(YAML::Emitter&, FieldData, char*)>> TypeSerializationMap;

	GNS_API std::string SerializeScene(gns::Scene* scene);
	GNS_API YAML::Emitter& SerializeField(YAML::Emitter& out, const FieldData& field, void* componentPtr);

	GNS_API static void RegisterSerializableComponents(const std::function<void()>& callback);
	/* 
	template<typename T>
	T* GetFieldValue(FieldData field, void* component_ptr)
	{
		char* cmpPtr = static_cast<char*>(component_ptr);
		T* t_ptr = static_cast<T*>(TypeSerializationMap[field.typeID](field, cmpPtr));
		return *t_ptr;
	}
	*/
};
