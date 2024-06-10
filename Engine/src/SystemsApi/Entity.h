﻿#pragma once
#include <entt/entt.hpp>
#include "api.h"
#include "SystemsAPI.h"
namespace gns
{
	class Scene;

	struct ComponentMetadata
	{
		void* data;
		uint32_t typehash;
	};

	struct ComponentBase;

	struct Entity
	{
		friend class Scene;

		entt::entity entity;
		Entity(entt::entity entity) :
			entity{ entity }
		{};

		Entity() = delete;

		inline bool IsValid() { return entity != entt::null; }

		operator bool() { return IsValid(); }

		template<typename T, typename... Args>
		T& AddComponet(Args&& ... args)
		{
			T& component = SystemsAPI::GetRegistry().emplace<T>(entity, std::forward<Args>(args)...);

			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			return SystemsAPI::GetRegistry().get<T>(entity);
		}

		template<typename T>
		bool TryGetComponent(T* component)
		{
			component = SystemsAPI::GetRegistry().try_get<T>(entity);
			return component != nullptr;
		}

		GNS_API const std::vector<gns::ComponentMetadata>& GetAllComponent ();

		/*
		Entity GetChild(std::string name);
		Entity GetChild(size_t index);
		void SetParent(Entity entity);
		void GetParent();
		 */
		private:
			std::vector<gns::ComponentMetadata> componentsVector;
			static Entity CreateEntity(std::string entityName, Scene* scene);
	};
}
