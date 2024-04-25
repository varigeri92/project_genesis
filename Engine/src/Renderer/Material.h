#pragma once
#include <vulkan/vulkan.h>
#include "Utils/Buffer.h"

namespace gns::rendering
{
	class Texture;
	class Shader;

	struct Material{

		GNS_API Material(std::shared_ptr<gns::rendering::Shader> shader, std::string name);
		~Material();
		VkDescriptorSet descriptorSet;
		std::string name;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Texture> m_texture{ nullptr };
		std::vector<std::shared_ptr<Texture>> m_textures;

		GNS_API void SetTexture(std::shared_ptr<Texture> texture, uint32_t binding);
	};
}
