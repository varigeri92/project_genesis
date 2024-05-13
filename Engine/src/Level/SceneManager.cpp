#include "gnspch.h"
#include "SceneManager.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

namespace gns::core
{
	struct SerializedEntityData
	{
		std::string Name;
		uint64_t Guid;
		std::vector<std::string> ComponentName;
		std::vector<uint32_t> ComponentID;
	};
	struct SerialzedSceneData
	{
		std::string name;
		std::vector<SerializedEntityData> entities;
	};

	Scene* SceneManager::ActiveScene = nullptr;
	Scene* SceneManager::CreateScene(std::string name)
	{
		Scene* newScene = new Scene();
		if (ActiveScene == nullptr)
			ActiveScene = newScene;

		IntegrateScene(newScene);
		return newScene;
	}

	Scene* SceneManager::SerializeScene(Scene* scene)
	{
		SerialzedSceneData sceneData = {};
		sceneData.name = scene->name;
		sceneData.entities = {};
		for (const auto e : scene->entities)
		{
			gns::Entity entity{ e };
			SerializedEntityData serializedEntity = {};
			serializedEntity.Name = entity.GetComponent<EntityComponent>().name;
			serializedEntity.Guid = entity.GetComponent<EntityComponent>().guid;
			const std::vector<ComponentMetadata>& components = entity.GetAllComponent();
			for (const auto& compdata : components)
			{
				serializedEntity.ComponentName.emplace_back(Serializer::ComponentData_Table[compdata.typehash].name);
				serializedEntity.ComponentID.emplace_back(Serializer::ComponentData_Table[compdata.typehash].typeID);
				
			}
		}

		YAML::Emitter out;
		out << YAML::BeginSeq;
		out << "eggs";
		out << "bread";
		out << "milk";
		out << YAML::EndSeq;

		LOG_INFO("Yaml result:\n" << out.c_str()<<"\n");
		return scene;
	}

	void SceneManager::IntegrateScene(Scene* scene)
	{

	}
}
