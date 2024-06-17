#include "gnspch.h"
#include "Material.h"

#include "RenderSystem.h"
#include "Utils/VkDebug.h"
#include "Shader.h"
#include "Texture.h"
#include "Utils/VkHelpers.h"

namespace gns::rendering
{
	Material::Material(Shader* shader, std::string name) :
		m_shader(shader), name(name)
	{
		m_textures.resize(5);
		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		//Texture set
		VkDescriptorSetAllocateInfo textureSetAllocInfo = {};
		textureSetAllocInfo.pNext = nullptr;
		textureSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		textureSetAllocInfo.descriptorPool = device->m_descriptorPool;
		textureSetAllocInfo.descriptorSetCount = 1;
		textureSetAllocInfo.pSetLayouts = &shader->shaderSetLayout;
		_VK_CHECK(vkAllocateDescriptorSets(device->m_device, &textureSetAllocInfo, &descriptorSet), "Descriptor set Allocation Failed!");


		LOG_INFO("Material Created! " << name);
		
		if(shader->fragmentShaderDataSize > 0)
			fragmentShaderData_memory = malloc(shader->fragmentShaderDataSize);
		else
			fragmentShaderData_memory = nullptr;
		

		if (shader->vertexShaderDataSize > 0)
			vertexShaderData_memory = malloc(shader->vertexShaderDataSize);
		else
			vertexShaderData_memory = nullptr;

		SetFragmentShaderAttribute<glm::vec4>("color_albedo", { 1.f,1.f,1.f,1.f });
		SetFragmentShaderAttribute<float>("specular", 0.5f);

		attributeBuffer = CreateBuffer(device->m_allocator, sizeof(glm::vec4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		constexpr uint32_t bufferRange = sizeof(glm::vec4);
		VkDescriptorBufferInfo bufferInfo = CreateBufferInfo(attributeBuffer._buffer, bufferRange, 0);
		VkWriteDescriptorSet data = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSet, &bufferInfo, 0);
		vkUpdateDescriptorSets(device->m_device, 1, &data, 0, nullptr);
	}

	Material::~Material()
	{
		free(vertexShaderData_memory);
		vertexShaderData_memory = nullptr;
		free(fragmentShaderData_memory);
		fragmentShaderData_memory = nullptr;
		RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
		renderSystem->DisposeMaterial(this);
		vmaDestroyBuffer(renderSystem->GetDevice()->m_allocator, attributeBuffer._buffer, attributeBuffer._allocation);
		LOG_INFO("Material Destroyed: " << name);
	}

	void Material::SetTexture(std::shared_ptr<Texture> texture, uint32_t binding)
	{
		binding++;
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

	void Material::Dispose()
	{
		Object::Dispose();
	}
}
