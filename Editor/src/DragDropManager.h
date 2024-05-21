#pragma once
#include "../../Engine/src/AssetDatabase/AssetDatabase.h"
#include "../../Engine/src/Utils/Guid.h"

namespace gns::editor
{
	struct Payload
	{
		AssetMetadata* metaData;
		std::string path;
	};

	class DragDropManager
	{
		static Payload* m_payload;
	public:
		static Payload* GetCurrentPayload();
		static void SetCurrentPayload(AssetMetadata* metadata, std::string path);
	};
}
