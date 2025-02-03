#include "gnspch.h"
#include "Object.h"

namespace gns
{
	Object* Object::gns_NullObject = new Object(-1); /*uint6t_t maxValue*/
	std::unordered_map<core::guid, Object*> Object::s_objectMap = {};
	void Object::Dispose()
	{
		delete s_objectMap[m_guid];
		s_objectMap[m_guid] = gns_NullObject;
	}

	void Object::DisposeAll()
	{
		for (auto it = s_objectMap.begin(); it != s_objectMap.end(); ++it) {
			if(IsValid(it->second))
				it->second->Dispose();
			else
			{
				LOG_WARNING("trying to dispose null_object!");
			}
				
		}
	}

	bool Object::IsValid(const Object* object)
	{
		return (object != gns_NullObject && object != nullptr);
	}
}
