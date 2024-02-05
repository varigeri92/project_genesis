#pragma once
#include <gnsAPI.h>
#include <memory>
#include <string>
#include <vector>
#include <glm/gtx/transform.hpp>
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
	struct Shader : public GnsObject
	{
		Shader(std::string vertexShaderPath, std::string fragmentShaderPath) : GnsObject(),
			vertexShaderPath(vertexShaderPath),
			fragmentShaderPath(fragmentShaderPath)
		{};
		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;

		Buffer materialUniformBuffer;
		VkDescriptorSet descriptorSet;
	};

	struct Material : public IDisposeable, public GnsObject {

		Material(std::shared_ptr<Shader> shader, std::string name) :m_shader(shader), name(name) {};

		std::string name;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Texture> m_texture{ nullptr };

		void GEN_API Dispose(Device* device) override;
		void GEN_API SetTexture(const std::shared_ptr<Texture>& texture);
	};
}

