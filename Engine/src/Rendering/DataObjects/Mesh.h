#pragma once
#include "../Helpers/Buffer.h"
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vma/vk_mem_alloc.h>
#include "DisposableVkObject.h"



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

	struct Mesh : public Disposeable {
		
	    std::vector<Vertex> _vertices;
	    Buffer _vertexBuffer;

		void Dispose(Device* device) override;
	};
}

