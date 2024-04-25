#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "api.h"

namespace gns::rendering
{
	class Device;

	class Buffer
	{
		public:
			VkBuffer _buffer;
			VmaAllocation _allocation;
			uint64_t debugID;
	};

	Buffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
}
