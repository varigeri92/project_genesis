#pragma once
#include "../Helpers/Buffer.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vma/vk_mem_alloc.h>
#include "DisposableVkObject.h"
#include <gnsAPI.h>
#include <string>


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

		static VertexInputDescription GetVertexDescription();
	};

	struct Mesh : public IDisposeable {

		std::string name;
		size_t materialIndex;
		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;
		Buffer _vertexBuffer;
		Buffer _indexBuffer;

		void GNS_API Dispose(Device* device) override;
	};
}

