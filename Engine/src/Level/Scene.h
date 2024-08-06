#pragma once
#include "../Utils/Guid.h"
#include "../../Engine/src/SystemsApi/Entity.h"

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
		GNS_API Entity& GetSceneRoot();
		GNS_API Entity GetEntityByGuid(const gns::core::guid guid);
	private:
		entt::registry m_registry;
		Entity m_sceneRoot;
		Entity SoftCreateEntity();
	};
}
