#pragma once
#include "Serializeable.h"
#include "api.h"
#include "../../Level/Scene.h"
namespace YAML
{
	class Node;
	class Emitter;
}

namespace gns
{
	class Serializer
	{
	public:

		GNS_API static std::unordered_map<uint32_t, ComponentData> ComponentData_Table;
		GNS_API static std::unordered_map<uint32_t, std::function<void* (Entity&)>> AddComponentMap;
		GNS_API static std::unordered_map<size_t, std::function<void(YAML::Emitter&, FieldData, char*)>> TypeSerializationMap;
		GNS_API static void RegisterSerializableComponents(const std::function<void()>& callback);



		GNS_API void* AddComponentByID(uint32_t cmp_id, Entity& entity);
		GNS_API std::string SerializeScene(gns::Scene* scene);
		GNS_API std::string DeserializeScene(const std::string& src);
		void* ReadFieldValue(size_t field_typeId, const YAML::Node& node);
		GNS_API YAML::Emitter& SerializeField(YAML::Emitter& out, const FieldData& field, void* componentPtr);

		/* 
		template<typename T>
		T* GetFieldValue(FieldData field, void* component_ptr)
		{
			char* cmpPtr = static_cast<char*>(component_ptr);
			T* t_ptr = static_cast<T*>(TypeSerializationMap[field.typeID](field, cmpPtr));
			return *t_ptr;
		}
		*/
	private:
		void* GetPointerToField(void* data_ptr, size_t offset);
		void* AddComponentFromSavedData(Entity& entity, uint32_t Component_typeId);
		void WriteFieldToComponent(void* component_ptr, const FieldData& fieldData, void* fieldValuer_ptr);
	};
}
