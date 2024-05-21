#pragma once
#include "../Utils/Guid.h"

namespace gns
{
	class Object
	{
		friend class Engine;
		static Object* gns_NullObject;
		GNS_API static std::unordered_map<core::guid, Object*> s_objectMap;
		core::guid m_guid;
		static void DisposeAll();
	public:
		GNS_API core::guid GetGuid() const { return m_guid; }

		GNS_API Object() :m_guid{ core::Guid::GetNewGuid() } {}
		Object(core::guid guid) :m_guid{ guid } {}
		GNS_API virtual ~Object() = default;
		GNS_API Object(const Object& other) = delete;
		
		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Create(Args&& ... args)
		{
			T* object = new T(std::forward<Args>(args)...);
			object->m_guid = core::Guid::GetNewGuid();
			s_objectMap[object->m_guid] = object;
			return object;
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Create(core::guid guid, Args&& ... args)
		{
			T* object = new T(std::forward<Args>(args)...);
			object->m_guid = guid;
			s_objectMap[object->m_guid] = object;
			return object;
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type>
		static T* Get(gns::core::guid guid)
		{
			if(!s_objectMap.contains(guid))
			{
				LOG_ERROR("Object Map does not contain Guid: " << guid);
			}
			if(IsValid(s_objectMap[guid]))
			{
				return static_cast<T*>(s_objectMap[guid]);
			}
			LOG_ERROR("The Object with guid: '" << guid << "' has beed already destoyed, or has never been created!");
			return nullptr;
		}

		GNS_API static bool IsValid(const Object* object);
		virtual void Dispose();
	};
}
