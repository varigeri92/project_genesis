#include "Material.h"

#include "Texture.h"
#include "../Device.h"
#include "../Helpers/VkInitializer.h"
#include <vklog.h>

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


	//allocate the descriptor set for single-m_texture to use on the material
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = RenderSystem::S_Device->m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_shader->shaderSetLayout;
	_VK_CHECK(vkAllocateDescriptorSets(RenderSystem::S_Device->m_device, &allocInfo, &texture->descriptorSet), "Descriptor set Allocation Failed!");

	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = texture->m_sampler;
	imageBufferInfo.imageView = texture -> imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture->descriptorSet,
		&imageBufferInfo, m_textures.size()-1);

	vkUpdateDescriptorSets(RenderSystem::S_Device->m_device, 1, &texture1, 0, nullptr);

	texture->Apply();
}

gns::rendering::Shader::~Shader()
{
	vkDestroyDescriptorSetLayout(RenderSystem::S_Device->m_device, shaderSetLayout, nullptr);
}
