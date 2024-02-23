#pragma once
#ifdef EDITOR_BUILD
#define SERIALIZE(type, name, value)type name = value;\
	SerializedProperty serialized_##name = {#type, #name, &name};\

#else
#define SERIALIZE(type, name, value) type name = value
#endif

#define COMPONENT(name) struct name : public gns::ComponentBase
