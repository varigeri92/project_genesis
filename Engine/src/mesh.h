#pragma once
#include "Rendering/Device.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include <vulkan/vulkan.h>
namespace gns
{
	struct Mesh
	{
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;
	
		Mesh(std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices)
		{
			Vertices = std::move(vertices);
			Indices = std::move(indices);
		}
	};
}