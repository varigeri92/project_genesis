#pragma once
#include "../SystemsApi/ComponentLibrary.h"

namespace YAML
{
	class Node;
	class Emitter;
}

namespace gns::core
{
	struct SceneEntity
	{
		std::vector<ComponentBase*> componentDataArray;
		std::vector<ComponentData> serializationInfos;
	};

	struct Scene
	{
		std::string name;
		std::vector<entt::entity> entities;
		void AddEntity(entt::entity entity);
	};

	class SceneManager
	{
	public:
		GNS_API static Scene* ActiveScene;
		GNS_API static Scene* LoadScene(std::string path);
		GNS_API static Scene* CreateScene(std::string name);
		GNS_API static Scene* SerializeScene(Scene* scene);

	private:
		static void IntegrateScene(Scene* scene);
		static void* GetPointerToField(void* data_ptr, size_t offset);
		static YAML::Emitter& WriteFieldValue(YAML::Emitter& out, void* data_ptr, size_t offset, size_t typeId);
		static void* ReadFieldValue(size_t field_typeId, const YAML::Node& node);
		static void* AddComponentFromSavedData(Entity& entity, uint32_t Component_typeId);
		static void WriteFieldToComponent(void* component_ptr, const FieldData& fieldData, const void* fieldValuer_ptr);
		static Scene* loadSceneFromFile(std::string path);
		static void SaveSceneToFile(std::string path);
	};
}
