#pragma once
#include "../Utils/Guid.h"

namespace gns
{
	class Serializer;

	class Scene
	{
		friend struct Entity;
		friend class Serializer;
	public:
		Scene(std::string name);
		std::string name;
		std::vector<Scene> m_subScenes;
	private:
		entt::registry m_registry;
		Entity SoftCreateEntity();
	public:
		GNS_API Entity CreateEntity(std::string name);
		GNS_API Entity FindEntity(std::string name);
		GNS_API Entity FindEntity(gns::core::guid name);
		GNS_API std::vector<Entity> GetEntities(gns::core::guid guid);

		template<typename T, typename ...>
		entt::view<T> GetView()
		{
			return m_registry.view<T ...>();
		}

		entt::registry& Registry();
	};
}
