#pragma once
#include <vulkan/vulkan.h>
#include "Utils/Buffer.h"
#include "../Object/Object.h"

namespace gns::rendering
{
	class Texture;
	class Shader;

	struct Material : public gns::Object
	{

		GNS_API Material(gns::rendering::Shader* shader, std::string name);
		GNS_API ~Material() override;
		VkDescriptorSet descriptorSet;
		std::string name;
		Shader* m_shader;
		std::shared_ptr<Texture> m_texture{ nullptr };
		std::vector<std::shared_ptr<Texture>> m_textures;

		GNS_API void SetTexture(std::shared_ptr<Texture> texture, uint32_t binding);
		void Dispose() override;
	};
}
