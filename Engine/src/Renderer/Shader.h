#pragma once
#include <string>
#include <vulkan/vulkan.h>

namespace gns::rendering
{
	class Renderer;

	class Shader
	{
		friend class Renderer;
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
	public:
		Shader() = delete;
		Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath);
		~Shader();

		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout shaderSetLayout;
	};
}
