#pragma once
#include <string>
#include <EnTT/entt.hpp>
#include <memory>
#include "../Core/Scene.h"

namespace gns {
	struct Entity
	{
		std::shared_ptr<Scene> scene;
		entt::entity entity;
		Entity(entt::entity entity, std::shared_ptr<Scene> scene) :
			entity	{ entity },
			scene	{ scene } 
		{};
		
		Entity() = default;

		template<typename T, typename... Args>
		T& AddComponet(Args&& ... args)
		{
			T& component = scene->registry.emplace<T>(entity, std::forward<Args>(args)...);
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
	};
}

