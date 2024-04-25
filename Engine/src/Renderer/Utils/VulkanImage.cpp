#include <gnspch.h>
#include "VulkanImage.h"
#include "../Device.h"

void gns::rendering::VulkanImage::Destroy(Device* device)
{
	vmaDestroyImage(device->m_allocator, _image, _allocation);
}
