#include "gnspch.h"
#include "Serializer.h"
#include "../../SystemsApi/ComponentLibrary.h"
#include "yaml-cpp/yaml.h"

std::unordered_map<uint32_t, ComponentData> Serializer::ComponentData_Table = {};
std::unordered_map<size_t, std::function<void(YAML::Emitter&, FieldData, char*)>> Serializer::TypeSerializationMap =
{ {typeid(float).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const float value = *reinterpret_cast<float*>(ptr += data.offset);
		out << value;
	}},
	{typeid(gns::core::guid).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const gns::core::guid value = *reinterpret_cast<gns::core::guid*>(ptr += data.offset);
		out<<value;
	}},
	{typeid(std::string).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const auto value = *reinterpret_cast<std::string*>(ptr += data.offset);
		out << value;
	}},
	{typeid(glm::vec3).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const auto value = *reinterpret_cast<glm::vec3*>(ptr += data.offset);
		out << YAML::Flow << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
	}},
	{typeid(gns::rendering::Mesh).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const auto value = reinterpret_cast<gns::rendering::Mesh**>(ptr += data.offset);
		gns::rendering::Mesh* m = *value;
		gns::core::guid guid = m->GetGuid();
		out << guid;
	}},
	{typeid(std::vector<gns::rendering::Material*>).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		out << 0;
	}}
};

std::string Serializer::SerializeScene(gns::Scene* scene)
{
	YAML::Emitter out;
	out << YAML::BeginMap << "version" << "0.2.0"
	<< "scene" << scene->name
	<< "entities" << YAML::BeginSeq;
	auto view = scene->Registry().view<gns::EntityComponent>();
	for (auto [entt, entity] : view.each())
	{
		gns::Entity e = gns::Entity(entt);
		auto comps = e.GetAllComponent();
		for (auto componentData : comps)
		{
			YAML::Emitter emitter;
			emitter << "hello";
			out << YAML::BeginMap
				<< "component_name" << ComponentData_Table[componentData.typehash].name
				<< "component_id" << ComponentData_Table[componentData.typehash].typeID
				<< "component_fields" << YAML::BeginSeq;
			for (FieldData field : ComponentData_Table[componentData.typehash].fields)
			{
				out << YAML::BeginMap
					<< "field_name" << field.name
					<< "field_type" << field.typeID
					<< "field_value";
				SerializeField(out, field, componentData.data) << YAML::EndMap;
			}
			out << YAML::EndSeq << YAML::EndMap;
			
		}
	}
	out << YAML::EndSeq << YAML::EndMap;
	std::string res = out.c_str();
	return res;
}

YAML::Emitter& Serializer::SerializeField(YAML::Emitter& out, const FieldData& field, void* componentPtr)
{
	char* cmpPtr = static_cast<char*>(componentPtr);
	if(TypeSerializationMap.contains(field.typeID))
	{
		TypeSerializationMap[field.typeID](out, field, cmpPtr);
		return out;
	}
	return out << "No_function_present_in_map";
}

void Serializer::RegisterSerializableComponents(const std::function<void()>& callback)
{
	callback();
}
