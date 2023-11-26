#include "Buffer.h"
#include "vklog.h"
#include "../Device.h"

namespace gns::rendering
{
	void Buffer::Dispose(Device* device)
	{
		vmaDestroyBuffer(device->m_allocator, _buffer, _allocation);
	}

	Buffer CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage,
	                            VmaMemoryUsage memoryUsage)
	{
		LOG_INFO("BufferAllocation Size: " << allocSize);
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;


		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = memoryUsage;

		Buffer newBuffer;

		_VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo,
			&newBuffer._buffer,
			&newBuffer._allocation,
			nullptr), "Bufferallocation Failed");

		return newBuffer;
	}
}
