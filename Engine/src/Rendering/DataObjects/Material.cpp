#include "Material.h"

#include "Texture.h"
#include "../Device.h"
#include "../Helpers/VkInitializer.h"

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

void gns::rendering::Material::PushTexture(const std::shared_ptr<Texture>& texture)
{
	m_textures.push_back(texture);

	if(m_textures.size() == 1)
		m_texture = texture;

	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = texture->m_sampler;
	imageBufferInfo.imageView = texture -> imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture->descriptorSet,
		&imageBufferInfo, m_textures.size()-1);

	vkUpdateDescriptorSets(RenderSystem::S_Device->m_device, 1, &texture1, 0, nullptr);

	texture->Apply();
}
