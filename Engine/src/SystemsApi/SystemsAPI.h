#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "SystemBase.h"
#include "api.h"
#include "entt/entt.hpp"
namespace gns
{
	class Engine;
	class SystemsAPI
	{
	private:
		friend class Engine;
		static entt::registry m_registry;

		static GNS_API std::vector<gns::SystemBase*> Systems;
		static void UpdateSystems(float deltaTime);
		static void ClearSystems();

		static int32_t GetIndexOfType(size_t typeHash)
		{
			for (int32_t i = 0; i < Systems.size(); i++)
			{
				if (Systems[i]->typeHash == typeHash)
				{
					return i;
				}
			}
			return -1;
		};
		static std::string GetSimpleName(std::string typeName) {
			size_t lastColonPos = typeName.rfind("::");
			if (lastColonPos != std::string::npos) {
				typeName = typeName.substr(lastColonPos + 2);
			}
			return typeName;
		}

	public:

		GNS_API static entt::registry& GetDefaultRegistry();
		GNS_API static entt::registry& GetRegistry(uint32_t registryIndex);


		template<typename T, typename... Args>
		static T* RegisterSystem(Args&& ... args)
		{
			size_t typehash = typeid(T).hash_code();
			if (GetIndexOfType(typehash) == -1)
			{
				T* newSystem = new T{ std::forward<Args>(args)... };
				Systems.push_back(newSystem);
				const size_t systemIndex = Systems.size() - 1;
				//Systems[systemIndex]->index = systemIndex;
				//Systems[systemIndex]->name = GetSimpleName(typeid(T).name());
				Systems[systemIndex]->typeHash = typehash;
				Systems[systemIndex]->OnCreate();
				return newSystem;
			}
			return nullptr;
		};
		template<typename T>
		static std::enable_if_t<std::is_base_of_v<SystemBase, T>, void> UnregisterSystem()
		{
			auto typehash = typeid(T).hash_code();
			int32_t index = GetIndexOfType(typehash);
			if (index > -1)
			{
				Systems.erase(Systems.begin() + index);
			}
		}

		template<typename T>
		static std::enable_if_t<std::is_base_of_v<SystemBase, T>, T*> GetSystem()
		{
			auto typehash = typeid(T).hash_code();
			int32_t index = GetIndexOfType(typehash);
			return dynamic_cast<T*>(Systems[index]);
		}

	};
}
