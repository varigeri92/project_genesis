#pragma once
#include "Log.h"
#include "SystemBase.h"
#include <type_traits>
namespace gns::core
{
	
class SystemsApi
{
private:
	static int32_t GetIndexOfType(size_t typeHash)
	{
		for (size_t i = 0; i < Systems.size(); i++)
		{
			if(Systems[i]->typeHash == typeHash)
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
	static GEN_API std::vector<gns::core::SystemBase*> Systems;
	template<typename T, typename... Args>
	static void RegisterSystem(Args&& ... args)
	{
		size_t typehash = typeid(T).hash_code();
		if (GetIndexOfType(typehash) == -1)
		{
			T* newSystem = new T { std::forward<Args>(args)... };
			Systems.push_back(newSystem);
			size_t systemIndex = Systems.size() - 1;
			Systems[systemIndex]->index = systemIndex;
			Systems[systemIndex]->name = GetSimpleName(typeid(T).name());
			Systems[systemIndex]->typeHash = typehash;
			Systems[systemIndex]->OnCreate();
		}
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

	static void GEN_API UpdateSystems();
};

}

