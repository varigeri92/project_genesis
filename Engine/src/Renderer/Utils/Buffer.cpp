#include <gnspch.h>
#include "Buffer.h"
#include "VkDebug.h"
#include "../Utils/Random.h"

gns::rendering::Buffer gns::rendering::CreateBuffer(VmaAllocator allocator, size_t allocSize, VkBufferUsageFlags usage,
                                                    VmaMemoryUsage memoryUsage)
{
	
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;


	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;

	Buffer newBuffer;
	newBuffer.debugID = Random::Get<uint64_t>();
	_VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo,
		&newBuffer._buffer,
		&newBuffer._allocation,
		nullptr), "Bufferallocation Failed");

	LOG_INFO("BufferAllocation Size: " << allocSize << " -- debugID: " << newBuffer.debugID);
	return newBuffer;
}
