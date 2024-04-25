#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace gns::rendering
{
	class Device;

	struct VulkanImage
	{
		VkImage _image;
		VmaAllocation _allocation;

		void Destroy(Device* device);
	};
}