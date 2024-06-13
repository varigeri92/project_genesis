#pragma once
#include <vulkan/vulkan.h>

#include "Shader.h"
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
		Buffer attributeBuffer;
		std::string name;
		Shader* m_shader;
		std::shared_ptr<Texture> m_texture{ nullptr };
		std::vector<std::shared_ptr<Texture>> m_textures;

		void* vertexShaderData_memory;
		void* fragmentShaderData_memory;

		std::unordered_map<std::string, size_t> attributeMapping;


		GNS_API void SetTexture(std::shared_ptr<Texture> texture, uint32_t binding);
		void Dispose() override;

		template<typename T>
		T* GetFragmentShaderAttribute(const std::string& attributeName)
		{
			bool found = false;
			char* memBlock_1stByte = static_cast<char*>(fragmentShaderData_memory);
			size_t offset = 0;
			for (auto & element : m_shader->m_fragmentShaderAttributes)
			{
				if (element.attributeName == attributeName)
				{
					offset = element.offset;
					break;
				}
			}
			char* selectedByte = memBlock_1stByte + offset;
			return static_cast<T*>((void*)selectedByte);
		}

		template<typename T>
		void SetFragmentShaderAttribute(const std::string& attributeName, const T& value)
		{
			char* memBlock_1stByte = static_cast<char*>(fragmentShaderData_memory);
			size_t offset = 0;
			for (auto& element : m_shader->m_fragmentShaderAttributes)
			{
				if (element.attributeName == attributeName)
				{
					offset = element.offset;
					break;
				}
			}
			char* selectedByte = memBlock_1stByte + offset;
			memcpy(selectedByte, &value, sizeof(T));
		}
	};
}
