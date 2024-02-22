#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include "../DataObjects/DisposableVkObject.h"

namespace gns::rendering
{
	class Renderer;
	struct Mesh;
	class Buffer : IDisposeable
	{
	public:
		VkBuffer _buffer;
		VmaAllocation _allocation;
		GNS_API void Dispose(Device* device) override;
	};

	struct VulkanImage {
		VkImage _image;
		VmaAllocation _allocation;
	};

	Buffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
}

