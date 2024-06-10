#include "gnspch.h"
#include "SceneManager.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

#include <filesystem>

#include "../AssetDatabase/AssetDatabase.h"
#include "../AssetDatabase/AssetLoader.h"
#include "../Renderer/Utils/Buffer.h"
#include "Scene.h"

namespace fs = std::filesystem;


namespace gns::core
{
	Scene* SceneManager::ActiveScene = nullptr;

	Scene* SceneManager::LoadScene(std::string path)
	{
		/*
		YAML::Node sceneNode = YAML::LoadFile(AssetLoader::GetAssetsPath() + "\\DefaultScene.gnsscene");
		LOG_INFO(AQUA << "[Scene Serializer]:" << DEFAULT << "Serialized vith version: " << sceneNode["version"].as<std::string>());
		LOG_INFO(AQUA << "[Scene Serializer]:" << DEFAULT << "Scene name: " << sceneNode["scene"].as<std::string>());
		Scene* scene = CreateScene(sceneNode["scene"].as<std::string>());

		const YAML::Node& entities = sceneNode["entities"];
		for (std::size_t i = 0; i < entities.size(); i++) {
			Entity entity = Entity::CreateEntity("New LoadedEntity", scene);
			for (std::size_t k = 0; k < entities[i].size(); k++)
			{
				const YAML::Node& component = entities[i][k];
				LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component name: " << component["component_name"].as<std::string>());
				LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component id: " << component["component_id"].as<uint32_t>());
				LOG_INFO(AQUA << "\t[Scene Serializer]:" << DEFAULT << "entity-component fields: ");
				void* component_ptr = AddComponentFromSavedData(entity, component["component_id"].as<uint32_t>());
				const YAML::Node& fields = component["component_fields"];
				for (std::size_t j = 0; j < fields.size(); j++)
				{
					LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_type_id: " << fields[j]["field_type_id"].as<size_t>());
					LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_offset: " << fields[j]["field_offset"].as<size_t>());
					LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_size: " << fields[j]["field_size"].as<size_t>());
					LOG_INFO(AQUA << "\t\t[Scene Serializer]:" << DEFAULT << "field_name: " << fields[j]["field_name"].as<std::string>());
					void* field_ptr = ReadFieldValue(fields[j]["field_type_id"].as<size_t>(), fields[j]["field_value"]);
					if (field_ptr != nullptr && component_ptr != nullptr)
					{
						LOG_INFO("Writing field data to: " << Serializer::ComponentData_Table[component["component_id"].as<uint32_t>()].fields[j].name);
						WriteFieldToComponent(component_ptr, 
							Serializer::ComponentData_Table[component["component_id"].as<uint32_t>()].fields[j],
							field_ptr);
					}
				}
				
			}
		}
		*/
		return nullptr;
	}

	Scene* SceneManager::CreateScene(std::string name)
	{
		Scene* newScene = new Scene(name);
		if (ActiveScene == nullptr)
			ActiveScene = newScene;
		return newScene;
	}

	Scene* SceneManager::SerializeScene(Scene* scene)
	{
		/*
		YAML::Emitter out;
		out << YAML::BeginMap << "version" << "0.0.1" <<"scene" << scene->name
		<< "entities" << YAML::BeginSeq;

		for (const auto e : scene->entities)
		{
			gns::Entity entity{ e };
			out << YAML::BeginSeq;
			const std::vector<ComponentMetadata>& components = entity.GetAllComponent();
			for (const auto& compdata : components)
			{
				out << YAML::BeginMap;
				out << "component_name" << Serializer::ComponentData_Table[compdata.typehash].name;
				out <<"component_id"<< compdata.typehash;
				out << "component_fields" << YAML::BeginSeq;
				for (const auto& field : Serializer::ComponentData_Table[compdata.typehash].fields)
				{
					out << YAML::BeginMap
						<< "field_type_id" << field.typeID
						<< "field_offset" << field.offset
						<< "field_size" << field.size
						<< "field_name" << field.name
						<< "field_value";
					WriteFieldValue(out, compdata.data, field.offset, field.typeID)
					<< YAML::EndMap;
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		LOG_INFO("Yaml result:\n" << out.c_str()<<"\n");
		writeFile(out.c_str(), AssetLoader::GetAssetsPath()+"\\"+scene->name+".gnsscene");
		*/
		return nullptr;
	}

	void SceneManager::IntegrateScene(Scene* scene)
	{

	}

	void* SceneManager::GetPointerToField(void* data_ptr, size_t offset)
	{
		char* ptr = static_cast<char*>(data_ptr);
		return (void*)(ptr += offset);
	}


	YAML::Emitter& SceneManager::WriteFieldValue(YAML::Emitter& out, void* data_ptr, size_t offset, size_t typeId)
	{
		if (typeId == typeid(float).hash_code())
		{
			return out << *(static_cast<float*>(GetPointerToField(data_ptr, offset)));
		}
		if (typeId == typeid(std::string).hash_code())
		{
			return out << *(static_cast<std::string*>(GetPointerToField(data_ptr, offset)));
		}
		if (typeId == typeid(glm::vec3).hash_code())
		{
			return out << YAML::Flow << YAML::BeginSeq << (static_cast<glm::vec3*>(GetPointerToField(data_ptr, offset)))->x <<
				(static_cast<glm::vec3*>(GetPointerToField(data_ptr, offset)))->y <<
				(static_cast<glm::vec3*>(GetPointerToField(data_ptr, offset)))->z << YAML::EndSeq;
		}
		if (typeId == typeid(gns::rendering::Mesh).hash_code())
		{
			gns::rendering::Mesh** mesh = static_cast<gns::rendering::Mesh**>(GetPointerToField(data_ptr, offset));
			gns::rendering::Mesh* m = *mesh;
			guid guid = m->GetGuid();
			return out << guid;
		}
		return out << "NaN";

	}

	void* SceneManager::ReadFieldValue(size_t field_typeId, const YAML::Node& node)
	{
		
		if(field_typeId == typeid(float).hash_code())
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
		return nullptr;
	}

	void* SceneManager::AddComponentFromSavedData(Entity& entity, uint32_t Component_typeId)
	{
		if(Component_typeId == entt::type_hash<gns::Transform, void>::value()) // Transform
		{
			Transform& component = entity.GetComponent<Transform>();
			return &component;
		}

		if (Component_typeId == entt::type_hash<gns::EntityComponent, void>::value()) // EntityComponent
		{
			gns::EntityComponent& component = entity.GetComponent<gns::EntityComponent>();
			return &component;
		}

		if (Component_typeId == entt::type_hash<gns::RendererComponent, void>::value()) // EntityComponent
		{
			gns::RendererComponent& component = entity.AddComponet<gns::RendererComponent>();
			return &component;
		}

		return nullptr;
	}

	void SceneManager::WriteFieldToComponent(void* component_ptr, const FieldData& fieldData,
		const void* fieldValuer_ptr)
	{
		LOG_INFO("Writing field "<< fieldData.name <<"Data to the Component");
		if(fieldData.typeID == typeid(gns::rendering::Mesh).hash_code())
		{
			guid guid;
			memcpy(&guid, fieldValuer_ptr, fieldData.size);
			AssetMetadata meshMeta = AssetDatabase::GetAssetByGuid(guid);
			gns::rendering::Mesh* mesh = Object::Get<rendering::Mesh>(meshMeta.guid);
			rendering::Material* material = nullptr;
			
			AssetMetadata default_materialMeta = AssetDatabase::GetAssetByName("__default_material");
			material = Object::Get<rendering::Material>(default_materialMeta.guid);
			for (const auto& subMesh : mesh->m_subMeshes)
			{
				static_cast<RendererComponent*>(component_ptr)->m_materials.push_back(material);
			}
		}
		else
		{
			void* value_ptr = GetPointerToField(component_ptr, fieldData.offset);
			memcpy(value_ptr, fieldValuer_ptr, fieldData.size);
		}
	}

	Scene* SceneManager::loadSceneFromFile(std::string path)
	{
		return nullptr;
	}

	void SceneManager::SaveSceneToFile(std::string path)
	{

	}
}
