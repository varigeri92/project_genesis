#pragma once
#include <entt/entt.hpp>
#include "api.h"
#include "SystemsAPI.h"
namespace gns
{
	struct ComponentMetadata
	{
		std::string name;
		void* data;
	};

	struct Entity
	{
		GNS_API static std::unordered_map<entt::entity, std::vector<ComponentMetadata>> ComponentRegistry;
		GNS_API static Entity CreateEntity(std::string entityName,uint32_t registry = 0);

		entt::entity entity;
		Entity(entt::entity entity) :
			entity{ entity }
		{};

		Entity() = delete;

		inline bool IsValid() { return entity != entt::null; }
		template<typename T, typename... Args>
		T& AddComponet(Args&& ... args)
		{
			T& component = SystemsAPI::GetDefaultRegistry().emplace<T>(entity, std::forward<Args>(args)...);
#ifdef EDITOR_BUILD
			ComponentRegistry[entity].emplace_back(typeid(T).name(), &component);
#endif
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			return SystemsAPI::GetDefaultRegistry().get<T>(entity);
		}

		template<typename T>
		bool TryGetComponent(T* component)
		{
			component = SystemsAPI::GetDefaultRegistry().try_get<T>(entity);
			return component != nullptr;
		}
		/*
		Entity GetChild(std::string name);
		Entity GetChild(size_t index);
		void SetParent(Entity entity);
		void GetParent();
		 */
	};
}
