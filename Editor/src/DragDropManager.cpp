#include "DragDropManager.h"


gns::editor::Payload* gns::editor::DragDropManager::m_payload = new Payload();

gns::editor::Payload* gns::editor::DragDropManager::GetCurrentPayload()
{
	return m_payload;
}

void gns::editor::DragDropManager::SetCurrentPayload_Asset(AssetMetadata* metadata, std::string path)
{
	m_payload->type = PayloadType::Asset;
	m_payload->metaData = metadata;
	m_payload->path = path;
}

void gns::editor::DragDropManager::SetCurrentPayload_Asset(std::string path)
{
	m_payload->type = PayloadType::Asset;
	m_payload->metaData = nullptr;
	m_payload->path = path;
}

void gns::editor::DragDropManager::SetCurrentPayload_Entity(uint32_t entityId)
{
	m_payload->type = PayloadType::Entity;
	m_payload->entity_id = entityId;
}
