#pragma once
#include "../../Engine/src/AssetDatabase/AssetDatabase.h"
#include "../../Engine/src/Utils/Guid.h"

namespace gns::editor
{
	enum class PayloadType
	{
		Entity, Asset, Component
	};
	struct Payload
	{
		PayloadType type;
		uint32_t entity_id;
		AssetMetadata* metaData;
		std::string path;
	};

	class DragDropManager
	{
		static Payload* m_payload;
	public:
		static Payload* GetCurrentPayload();
		static void SetCurrentPayload_Asset(AssetMetadata* metadata, std::string path);
		static void SetCurrentPayload_Asset(std::string path);
		static void SetCurrentPayload_Entity(uint32_t entityId);
	};
}
