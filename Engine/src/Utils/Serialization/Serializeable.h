#pragma once
#include <string>
#include <entt/entt.hpp>
#define SERIALIZE(type ,name) type name
#define SERIALIZE_RO(type ,name) type name


#define REGISTER_CMP(cmp)\
	Serializer::ComponentData_Table[(uint32_t)entt::type_hash<cmp>::value()] = { (uint32_t)entt::type_hash<cmp>::value(), sizeof(cmp) ,#cmp, {} };\
	using __cmp = cmp

#define REGISTER_FIELD(type ,name)\
	Serializer::ComponentData_Table[(uint32_t)entt::type_hash<__cmp>::value()].Add(typeid(type).hash_code(), offsetof(__cmp, name), #name, false)

#define REGISTER_FIELD_RO(type ,name)\
	Serializer::ComponentData_Table[(uint32_t)entt::type_hash<__cmp>::value()].Add(typeid(type).hash_code(), offsetof(__cmp, name), #name, true)


struct FieldData
{
	size_t typeID;
	size_t offset;
	std::string name;
	bool editorReadOnly;
};

struct ComponentData
{
	uint32_t typeID;
	size_t size;
	std::string name;
	std::vector<FieldData> fields;

	void Add(size_t typeID, size_t offset, std::string name, bool editorReadOnly)
	{
		LOG_INFO(name << " -->> " << typeID);
		fields.emplace_back(typeID, offset, name, editorReadOnly);
	}
};
