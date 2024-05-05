#pragma once
#include <entt/entt.hpp>
#include "api.h"
#include "SystemsAPI.h"
namespace gns
{
	struct ComponentMetadata
	{
		void* data;
		uint32_t typehash;
	};

	struct ComponentBase;

	struct Entity
	{
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

		GNS_API std::vector<gns::ComponentMetadata>GetAllComponent();

		/*
		Entity GetChild(std::string name);
		Entity GetChild(size_t index);
		void SetParent(Entity entity);
		void GetParent();
		 */
	};
}
