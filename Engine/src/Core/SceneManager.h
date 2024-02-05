#pragma once
#include <map>
#include <string>
#include "gnsAPI.h"
#include "../AssetDatabase/Guid.h"

namespace gns
{
	struct Scene;
}

namespace gns
{
	class SceneManager
	{
	private:
		static std::map<gns::core::guid, std::shared_ptr<Scene>> Scenes;
		static gns::core::guid ActiveSceneGuid;
	public:

		static GEN_API Scene* LoadScene(std::string path);
		static GEN_API Scene* LoadScene(gns::core::guid guid);
		static GEN_API Scene* GetActiveScene();

		static GEN_API Scene* CreateScene(std::string name);
	};
}
