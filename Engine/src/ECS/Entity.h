#pragma once
#include <string>
#include <EnTT/entt.hpp>
#include <memory>

#include "Log.h"
#include "../Core/Scene.h"
#include "../Core/Components.h"

#define EDITOR_BUILD
namespace gns {


	struct ComponentMetadata
	{
		const char* name;
		void* data;
	};

	struct Entity
	{
		inline static std::unordered_map<entt::entity, std::vector<ComponentMetadata>>
			ComponentRegistry = {{}};
		Scene* scene;
		entt::entity entity;
		Entity(entt::entity entity, Scene* scene) :
			entity	{ entity },
			scene	{ scene } 
		{};
		
		Entity() = delete;

		template<typename T, typename... Args>
		T& AddComponet(Args&& ... args)
		{
			T& component = scene->registry.emplace<T>(entity, std::forward<Args>(args)...);
#ifdef EDITOR_BUILD
			LOG_INFO("Component " << typeid(T).name() << " Was added to entity");
			ComponentRegistry[entity].emplace_back(typeid(T).name(), static_cast<void*>(&component));
#endif
			return component;
		}


		template<typename T>
		T& GetComponent()
		{
			return scene->registry.get<T>(entity);
		}

		template<typename T>
		bool TryGetComponent(T& component)
		{
			component = scene->registry.get<T>(entity);
			return true;
		}

		Transform& GetTransform()
		{
			return GetComponent<Transform>();
		}
	};
}

