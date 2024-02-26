#pragma once
#include <gnsAPI.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include "vulkan/vulkan.h"
#include "DisposableVkObject.h"
#include "../../AssetDatabase/Guid.h"
#include "../../Core/GnsObject.h"
#include "../Helpers/Buffer.h"

namespace gns::rendering
{
	class Texture;
}

namespace gns::rendering
{
	class Mesh;
	struct ShaderMetadata
	{
		uint32_t textureCount;
	};

	struct Shader : public GnsObject
	{
		GNS_API Shader(std::string vertexShaderPath, std::string fragmentShaderPath) : GnsObject(),
			vertexShaderPath(vertexShaderPath),
			fragmentShaderPath(fragmentShaderPath)
		{};
		GNS_API ~Shader();

		ShaderMetadata meta = {};
		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		Buffer shaderUniformBuffer;
		VkDescriptorSetLayout shaderSetLayout;
	};

	struct Material : public IDisposeable, public GnsObject {

		GNS_API Material(std::shared_ptr<Shader> shader, std::string name);

		VkDescriptorSet descriptorSet;
		std::string name;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Texture> m_texture{ nullptr };
		std::vector<std::shared_ptr<Texture>> m_textures;
		
		void GNS_API Dispose(Device* device) override;
		void GNS_API SetTexture(const std::shared_ptr<Texture>& texture);
		void GNS_API PushTexture(const std::shared_ptr<Texture>& texture);

	};
}

