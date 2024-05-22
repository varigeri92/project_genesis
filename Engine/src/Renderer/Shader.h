#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "../Object/Object.h"
#include <unordered_map>

namespace gns::rendering
{
	class Renderer;
	class Shader :public gns::Object
	{
		enum class ShaderAttributeType
		{
			Float, Float2, Float3, Flaot4, Mat4, Color, Color4,
			HdrColor4, Int, Flag, Texture
		};
		struct AttributeInfo
		{
			ShaderAttributeType type;
			uint32_t set;
			uint32_t binding;
		}; 
		friend class Renderer;
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
	public:
		Shader() = delete;
		GNS_API Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath);
		GNS_API ~Shader() = default;
		void Dispose() override;
		void ReadAttributes();

		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		std::unordered_map<std::string, AttributeInfo> m_vertexShaderAttributes;
		std::unordered_map<std::string, AttributeInfo> m_fragmentShaderAttributes;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout shaderSetLayout;
	};
}
