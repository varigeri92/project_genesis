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
	public:
		enum class ShaderAttributeType
		{
			Float, Float2, Float3, Float4, Mat4, Color, Color4,
			HdrColor4, Int, Flag, Texture
		};
		struct AttributeInfo
		{
			std::string attributeName;
			ShaderAttributeType type;
			size_t offset;
			uint32_t set;
			uint32_t binding;
		};
	private:
		friend class Renderer;
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
		void ReadAttributes();
	public:
		Shader() = delete;
		GNS_API Shader(const std::string vertexShaderPath, const std::string fragmentShaderPath);
		GNS_API ~Shader() = default;
		void Dispose() override;

		std::string vertexShaderPath;
		std::string fragmentShaderPath;

		std::vector<AttributeInfo> m_vertexShaderAttributes;
		std::vector<AttributeInfo> m_fragmentShaderAttributes;

		size_t fragmentShaderDataSize;
		size_t vertexShaderDataSize;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout shaderSetLayout;
	};
}
