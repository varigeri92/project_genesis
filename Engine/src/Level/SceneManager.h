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
		std::vector<entt::entity> entities;
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
