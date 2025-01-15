#include "gnspch.h"
#include "Serializer.h"

#include "../../AssetDatabase/AssetLoader.h"
#include "../../Level/SceneManager.h"
#include "../../SystemsApi/ComponentLibrary.h"
#include "yaml-cpp/yaml.h"
#include "../../src/Renderer/Lights/Lights.h"

static std::string currentVersion = "0.5.0";
namespace gns
{
std::unordered_map<uint32_t, ComponentData> Serializer::ComponentData_Table = {};
std::unordered_map<uint32_t, std::function<void* (Entity&)>> Serializer::AddComponentMap = {
	{entt::type_hash<EntityComponent>().value(),[](Entity& entity)
		{ return &entity.AddComponet<EntityComponent>(); }},
	{entt::type_hash<Transform>().value(),[](Entity& entity)
	{ 
		Transform& transform = entity.AddComponet<Transform>();
		entity.SetParent(core::SceneManager::ActiveScene->GetSceneRoot().entity);
		return &transform;
	}},
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
		const auto value = reinterpret_cast<std::vector<gns::rendering::Material*>*>(ptr += data.offset);
		std::vector<gns::rendering::Material*> m = *value;
		out << YAML::Flow << YAML::BeginSeq;
		for (size_t i = 0; i< m.size(); i++ )
		{
			gns::core::guid guid = m[i]->GetGuid();
			out << guid;
			LOG_INFO("Serialize_material_field: " << guid);
		}
		out << YAML::EndSeq;
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
	out << YAML::BeginMap << "version" << currentVersion
	<< "scene" << scene->name
	<< "entities" << YAML::BeginSeq;
	auto view = scene->Registry().view<gns::EntityComponent>();
	for (auto [entt, entity] : view.each())
	{
		if (entity.name == "SceneRoot") continue;

		gns::Entity e = gns::Entity(entt);
		out << YAML::BeginSeq;
		auto comps = e.GetAllComponent();
		for (auto componentData : comps)
		{
			LOG_INFO("Serilize component name: " << ComponentData_Table[componentData.typehash].name);
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
	if(root["version"].as<std::string>() != currentVersion)
	{
		LOG_ERROR("Serializer Version mismatch!");
		return "";
	}
	LOG_INFO(AQUA << "[Scene Serializer]:" << DEFAULT << "Serialized vith version: " << root["version"].as<std::string>());
	Scene* scene = gns::core::SceneManager::ActiveScene;

	const YAML::Node& entities = root["entities"];
	for (std::size_t i = 0; i < entities.size(); i++)
	{
		gns::Entity deserializedEntity = scene->SoftCreateEntity();
		for (std::size_t j = 0; j < entities[i].size(); j++)
		{
			const YAML::Node& component = entities[i][j];
			uint32_t componentID = component["component_id"].as<uint32_t>();
			void* component_ptr = AddComponentFromSavedData(deserializedEntity, componentID);
			const YAML::Node& fields = component["component_fields"];
			LOG_INFO(AQUA << "Reading Component:" << component["component_name"].as<std::string>() << ", field Count: " << fields.size());
			for (std::size_t k = 0; k < fields.size(); k++)
			{
				void* field_ptr = ReadFieldValue(fields[k]["field_type"].as<size_t>(), fields[k]["field_value"]);
				if(component_ptr != nullptr && field_ptr != nullptr)
				{
					WriteFieldToComponent(component_ptr, ComponentData_Table[componentID].fields[k], field_ptr, fields[k]["field_value"]);
				}
				else
				{
					if(field_ptr == nullptr)
						LOG_ERROR("Field pointer is 'nullptr' ...");
					else if(component_ptr == nullptr)
						LOG_ERROR("Component pointer is 'nullptr' ...");

				}
			}
		}
	}
	auto view = scene->Registry().view<gns::EntityComponent, Transform>();
	for (auto [entt, entity, transform] : view.each())
	{
		Entity(entt).SetParent(scene->GetEntityByGuid(transform.parent_guid).entity);
	}

	return "";
}

void* Serializer::ReadFieldValue(size_t field_typeId, const YAML::Node& node)
{

	if (field_typeId == typeid(float).hash_code())
	{
		float* f = new float(node.as<float>());
		return f;
	}
	if (field_typeId == typeid(glm::vec3).hash_code())
	{
		glm::vec3* vector = new glm::vec3{};
		for (std::size_t i = 0; i < node.size(); i++)
		{
			(*vector)[i] = node[i].as<float>();
		}
		return vector;
	}

	if (field_typeId == typeid(Color).hash_code())
	{
		Color* col = new Color{};
		for (std::size_t i = 0; i < node.size(); i++)
		{
			(*col)[i] = node[i].as<float>();
		}
		return col;
	}

	if (field_typeId == typeid(std::string).hash_code())
	{
		std::string* string = new std::string(node.as<std::string>());
		return string;
	}

	if (field_typeId == typeid(gns::rendering::Mesh).hash_code())
	{
		gns::core::guid* guid = new gns::core::guid(node.as<uint64_t>());
		return guid;
	}
	if (field_typeId == typeid(std::vector<rendering::Material*>).hash_code())
	{
		std::vector<gns::rendering::Material*>* field_ptr = new std::vector<gns::rendering::Material*>();
		return field_ptr;
	}

	if (field_typeId == typeid(gns::core::guid).hash_code())
	{
		gns::core::guid* guid = new gns::core::guid(node.as<uint64_t>());
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
	void* fieldValuer_ptr, const YAML::Node& node)
{
	LOG_INFO(GREEN << "Writing field '" << fieldData.name << "' Data to the Component");
	if (fieldData.typeID == typeid(gns::rendering::Mesh).hash_code())
	{
		LOG_INFO(GREEN << "Mesh!");

		core::guid guid;
		memcpy(&guid, fieldValuer_ptr, fieldData.size);

		auto* mesh = AssetLoader::LoadAssetFromFile<rendering::Mesh>(guid);
		SystemsAPI::GetSystem<RenderSystem>()->UploadMesh(mesh);
		RendererComponent* renderer = static_cast<RendererComponent*>(component_ptr);
		renderer->AssignMesh(guid);

	}
	else if(fieldData.typeID == typeid(std::vector<rendering::Material*>).hash_code())
	{

		LOG_INFO(GREEN << "Material!");
		RendererComponent* renderer = static_cast<RendererComponent*>(component_ptr);
		for (std::size_t i = 0; i < node.size(); i++)
		{
			core::guid guid = node[i].as<uint64_t>();
			AssetLoader::LoadAssetFromFile<rendering::Material>(guid);
			renderer->AssignMaterial(guid);
		}
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