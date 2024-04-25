#pragma once
#include "RenderSystem.h"
#include "Utils/Buffer.h"


namespace gns::rendering
{
	class Device;


	struct VertexInputDescription {

		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex {

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static VertexInputDescription GetVertexDescription()
		{
			VertexInputDescription description;

			//we will have just 1 vertex buffer binding, with a per-vertex rate
			VkVertexInputBindingDescription mainBinding = {};
			mainBinding.binding = 0;
			mainBinding.stride = sizeof(Vertex);
			mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			description.bindings.push_back(mainBinding);

			//Position will be stored at Location 0
			VkVertexInputAttributeDescription positionAttribute = {};
			positionAttribute.binding = 0;
			positionAttribute.location = 0;
			positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			positionAttribute.offset = offsetof(Vertex, position);

			//Normal will be stored at Location 1
			VkVertexInputAttributeDescription normalAttribute = {};
			normalAttribute.binding = 0;
			normalAttribute.location = 1;
			normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			normalAttribute.offset = offsetof(Vertex, normal);

			//Color will be stored at Location 2
			VkVertexInputAttributeDescription colorAttribute = {};
			colorAttribute.binding = 0;
			colorAttribute.location = 2;
			colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			colorAttribute.offset = offsetof(Vertex, color);

			VkVertexInputAttributeDescription uvAttribute = {};
			uvAttribute.binding = 0;
			uvAttribute.location = 3;
			uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
			uvAttribute.offset = offsetof(Vertex, uv);

			description.attributes.push_back(positionAttribute);
			description.attributes.push_back(normalAttribute);
			description.attributes.push_back(colorAttribute);
			description.attributes.push_back(uvAttribute);
			return description;
		}
	};

	struct Mesh
	{
		std::string name;
		size_t materialIndex;
		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;
		Buffer _vertexBuffer;
		Buffer _indexBuffer;
		Mesh()
		{
			LOG_INFO("Mesh Constructed:");
		};
		~Mesh();
	};

	inline Mesh::~Mesh()
	{
		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		device->DisposeBuffer(_vertexBuffer);
		device->DisposeBuffer(_indexBuffer);
		LOG_INFO("Mesh destructed:");
	}
}
