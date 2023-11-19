#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace gns::rendering
{
	class Renderer;
	struct Mesh;
	class Buffer
	{
		friend struct Mesh;
		friend class Renderer;
		VkBuffer _buffer;
		VmaAllocation _allocation;
	};

	struct VulkanImage {
		VkImage _image;
		VmaAllocation _allocation;
	};
}

