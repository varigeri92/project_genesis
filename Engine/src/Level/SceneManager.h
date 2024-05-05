#pragma once
#include "../SystemsApi/ComponentLibrary.h"

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
		std::vector<SceneEntity> sceneEntities;
		std::vector<entt::entity> loadedEntities;
	};

	class SceneManager
	{
	public:
		static Scene* ActiveScene;
		GNS_API static Scene* LoadScene(std::string path);
		GNS_API static Scene* CreateScene(std::string name);
		GNS_API static Scene* SerializeScene(Scene* scene);

	private:
		static void IntegrateScene(Scene* scene);
	};
}
