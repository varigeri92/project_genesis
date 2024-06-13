#include "gnspch.h"
#include "Serializer.h"

#include "../../AssetDatabase/AssetLoader.h"
#include "../../Level/SceneManager.h"
#include "../../SystemsApi/ComponentLibrary.h"
#include "yaml-cpp/yaml.h"
#include "../../src/Renderer/Lights/Lights.h"

namespace gns
{
	
std::unordered_map<uint32_t, ComponentData> Serializer::ComponentData_Table = {};
std::unordered_map<uint32_t, std::function<void* (Entity&)>> Serializer::AddComponentMap = {
	{entt::type_hash<EntityComponent>().value(),[](Entity& entity)
		{ return &entity.AddComponet<EntityComponent>(); }},
	{entt::type_hash<Transform>().value(),[](Entity& entity)
		{ return &entity.AddComponet<Transform>(); }},
	{entt::type_hash<RendererComponent>().value(),[](Entity& entity)
		{ return &entity.AddComponet<RendererComponent>(); }},
	{entt::type_hash<Camera>().value(),[](Entity& entity)
		{ return &entity.AddComponet<Camera>(); }},

	{entt::type_hash<Light>().value(),[](Entity& entity)
		{ return &entity.AddComponet<Light>(); }},
	{entt::type_hash<SpotLight>().value(),[](Entity& entity)
		{ return &entity.AddComponet<SpotLight>(); }},
	{entt::type_hash<PointLight>().value(),[](Entity& entity)
		{ return &entity.AddComponet<PointLight>(); }},
	{entt::type_hash<LightDirection>().value(),[](Entity& entity)
		{ return &entity.AddComponet<LightDirection>(); }},
	{entt::type_hash<Ambient>().value(),[](Entity& entity)
		{ return &entity.AddComponet<Ambient>(); }},
};
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
	}},
	{typeid(Color).hash_code(),
	[](YAML::Emitter& out, FieldData data, char* ptr)
	{
		const auto value = *reinterpret_cast<Color*>(ptr += data.offset);
		out << YAML::Flow << YAML::BeginSeq << value.r << value.g << value.b << YAML::EndSeq;
	}}
};
void* Serializer::AddComponentByID(uint32_t cmp_id, Entity& entity)
{
	if (AddComponentMap.contains(cmp_id))
		return AddComponentMap[cmp_id](entity);
	return nullptr;
}

std::string Serializer::SerializeScene(gns::Scene* scene)
{
	YAML::Emitter out;
	out << YAML::BeginMap << "version" << "0.3.0"
	<< "scene" << scene->name
	<< "entities" << YAML::BeginSeq;
	auto view = scene->Registry().view<gns::EntityComponent>();
	for (auto [entt, entity] : view.each())
	{
		gns::Entity e = gns::Entity(entt);
		out << YAML::BeginSeq;
		auto comps = e.GetAllComponent();
		for (auto componentData : comps)
		{
			out << YAML::BeginMap
				<< "component_name" << ComponentData_Table[componentData.typehash].name
				<< "component_id" << ComponentData_Table[componentData.typehash].typeID
				<< "component_fields" << YAML::BeginSeq;
			for (FieldData field : ComponentData_Table[componentData.typehash].fields)
			{
				out << YAML::BeginMap
					<< "field_name" << field.name
					<< "field_type" << field.typeID
					<< "field_offset" << field.offset
					<< "field_size" << field.size
					<< "field_value";
				SerializeField(out, field, componentData.data) << YAML::EndMap;
			}
			out << YAML::EndSeq << YAML::EndMap;
			
		}
		out << YAML::EndSeq;
	}
	out << YAML::EndSeq << YAML::EndMap;
	std::string res = out.c_str();
	return res;
}

std::string Serializer::DeserializeScene(const std::string& src)
{
	YAML::Node root = YAML::LoadFile(src);
	LOG_INFO(AQUA << "[Scene Serializer]:" << DEFAULT << "Serialized vith version: " << root["version"].as<std::string>());
	LOG_INFO(AQUA << "[Scene Serializer]:" << DEFAULT << "Scene name: " << root["scene"].as<std::string>());

	const YAML::Node& entities = root["entities"];
	for (std::size_t i = 0; i < entities.size(); i++)
	{
		gns::Entity deserializedEntity = gns::core::SceneManager::ActiveScene->SoftCreateEntity();
		for (std::size_t j = 0; j < entities[i].size(); j++)
		{
			const YAML::Node& component = entities[i][j];
			LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component name: " << component["component_name"].as<std::string>());
			uint32_t componentID = component["component_id"].as<uint32_t>();
			LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component id: " << componentID);
			void* component_ptr = AddComponentFromSavedData(deserializedEntity, componentID);

			LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component fields: ");
			const YAML::Node& fields = component["component_fields"];
			for (std::size_t k = 0; k < fields.size(); k++)
			{
				LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_name: " << fields[k]["field_name"].as<std::string>());
				LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_type_id: " << fields[k]["field_type"].as<size_t>());
				LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_offset: " << fields[k]["field_offset"].as<size_t>());
				LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_size: " << fields[k]["field_size"].as<size_t>());
				void* field_ptr = ReadFieldValue(fields[k]["field_type"].as<size_t>(), fields[k]["field_value"]);
				if(component_ptr != nullptr && field_ptr != nullptr)
				{
					WriteFieldToComponent(component_ptr, ComponentData_Table[componentID].fields[k], field_ptr);
				}
			}
		}
		
	}
	return "";
}

void* Serializer::ReadFieldValue(size_t field_typeId, const YAML::Node& node)
{

	if (field_typeId == typeid(float).hash_code())
	{
		float* f = new float(node.as<float>());
		LOG_INFO("field has the value of: " << f);
		return f;

	}
	if (field_typeId == typeid(glm::vec3).hash_code())
	{
		glm::vec3* vector = new glm::vec3{};
		for (std::size_t i = 0; i < node.size(); i++)
		{
			(*vector)[i] = node[i].as<float>();
		}
		LOG_INFO("field has the value of: " << vector->x << "/" << vector->y << "/" << vector->z);
		return vector;
	}

	if (field_typeId == typeid(Color).hash_code())
	{
		Color* col = new Color{};
		for (std::size_t i = 0; i < node.size(); i++)
		{
			(*col)[i] = node[i].as<float>();
		}
		LOG_INFO("field has the value of: " << col->r << "/" << col->g << "/" << col->b);
		return col;
	}

	if (field_typeId == typeid(std::string).hash_code())
	{
		std::string* string = new std::string(node.as<std::string>());
		LOG_INFO("field has the value of: " << node.as<std::string>());
		return string;
	}

	if (field_typeId == typeid(gns::rendering::Mesh).hash_code())
	{
		gns::core::guid* guid = new gns::core::guid(node.as<uint64_t>());
		LOG_INFO("field has the value of: " << node.as<std::string>());
		return guid;
	}

	if (field_typeId == typeid(gns::core::guid).hash_code())
	{
		gns::core::guid* guid = new gns::core::guid(node.as<uint64_t>());
		LOG_INFO("field has the value of: " << node.as<std::string>());
		return guid;
	}

	return nullptr;
}

void* Serializer::GetPointerToField(void* data_ptr, size_t offset)
{
	char* ptr = static_cast<char*>(data_ptr);
	return (void*)(ptr += offset);
}

void* Serializer::AddComponentFromSavedData(Entity& entity, uint32_t Component_typeId)
{
	if(AddComponentMap.contains(Component_typeId))
		return AddComponentMap[Component_typeId](entity);
	return nullptr;
}

void Serializer::WriteFieldToComponent(void* component_ptr, const FieldData& fieldData,
	void* fieldValuer_ptr)
{
	LOG_INFO("Writing field " << fieldData.name << "Data to the Component");
	if (fieldData.typeID == typeid(gns::rendering::Mesh).hash_code())
	{
		core::guid guid;
		memcpy(&guid, fieldValuer_ptr, fieldData.size);

		auto* mesh = AssetLoader::LoadAssetFromFile<rendering::Mesh>(guid);
		SystemsAPI::GetSystem<RenderSystem>()->UploadMesh(mesh);
		RendererComponent* renderer = static_cast<RendererComponent*>(component_ptr);
		renderer->AssignMesh(guid);
		renderer->AssignMaterial(0);
	}
	else
	{
		void* value_ptr = GetPointerToField(component_ptr, fieldData.offset);
		memcpy(value_ptr, fieldValuer_ptr, fieldData.size);
	}
	free(fieldValuer_ptr);
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
}