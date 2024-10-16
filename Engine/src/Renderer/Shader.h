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
		std::map<uint32_t, ShaderAttributeType>attributeTypeMap = {
			{9,ShaderAttributeType::Float4},
			{10,ShaderAttributeType::Float3},
			{57,ShaderAttributeType::Float2},
			{6,ShaderAttributeType::Float},
			{21,ShaderAttributeType::Int},
		};

		struct AttributeInfo
		{
			std::string attributeName;
			ShaderAttributeType type;
			size_t offset;
			uint32_t set;
			uint32_t binding;
		};
		const std::unordered_map<uint32_t, ShaderAttributeType> typeMap{ 
			std::pair<uint32_t, ShaderAttributeType>{9, ShaderAttributeType::Color4},
			std::pair<uint32_t, ShaderAttributeType>{10, ShaderAttributeType::Float3},
		};
	private:
		friend class Renderer;
		std::string m_vertexShaderPath;
		std::string m_fragmentShaderPath;
		void ReadAttributes();
		ShaderAttributeType GetAttributeType(std::string& name, uint32_t typeID);
		void AddAttribute(std::string& name, size_t offset, uint32_t type_ID, uint32_t set, uint32_t binding);
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
