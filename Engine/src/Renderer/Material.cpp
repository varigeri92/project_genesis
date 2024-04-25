#include "gnspch.h"
#include "Material.h"

#include "RenderSystem.h"
#include "Utils/VkDebug.h"
#include "Shader.h"
#include "Texture.h"
#include "Utils/VkHelpers.h"

namespace gns::rendering
{
	Material::Material(std::shared_ptr<Shader> shader, std::string name) :
		m_shader(shader), name(name)
	{
		m_textures.resize(5);
		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device->m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &shader->shaderSetLayout;
		_VK_CHECK(vkAllocateDescriptorSets(device->m_device, &allocInfo, &descriptorSet), "Descriptor set Allocation Failed!");
		LOG_INFO("Material Created! " << name);
	}

	Material::~Material()
	{
		RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
		renderSystem->DisposeMaterial(this);
		LOG_INFO("Material Destroyed: " << name);
	}

	void Material::SetTexture(std::shared_ptr<Texture> texture, uint32_t binding)
	{
		m_textures[binding] = texture;

		if (binding == 1)
			m_texture = texture;

		VkDescriptorImageInfo imageBufferInfo;
		imageBufferInfo.sampler = texture->m_sampler;
		imageBufferInfo.imageView = texture->m_imageView;
		imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet texture1 = WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet,
			&imageBufferInfo, binding);

		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		vkUpdateDescriptorSets(device->m_device, 1, &texture1, 0, nullptr);

		texture->Apply();
	}
}