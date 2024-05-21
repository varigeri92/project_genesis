#include "DragDropManager.h"


gns::editor::Payload* gns::editor::DragDropManager::m_payload = new Payload();

gns::editor::Payload* gns::editor::DragDropManager::GetCurrentPayload()
{
	return m_payload;
}

void gns::editor::DragDropManager::SetCurrentPayload(AssetMetadata* metadata, std::string path)
{
	
	m_payload->metaData = metadata;
	m_payload-> path = path;
}
