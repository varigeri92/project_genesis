#include "Material.h"
#include "../Device.h"

void gns::rendering::Material::Dispose(Device* device)
{
	/*
	vkDestroyPipeline(device->m_device, pipeline, nullptr);
	vkDestroyPipelineLayout(device->m_device, pipelineLayout, nullptr);
	*/
}

void gns::rendering::Material::SetTexture(const std::shared_ptr<Texture>& texture)
{
	m_texture = texture;
	
}
