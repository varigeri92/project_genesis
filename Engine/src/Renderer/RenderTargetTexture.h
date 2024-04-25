#pragma once
#include <vulkan/vulkan.h>
#include "Utils/VkDebug.h"

#include "Utils/VulkanImage.h"

namespace gns::rendering
{
class RenderTargetTexture
{
public:
	RenderTargetTexture() = default;
	void CreateRenderTarget();

	VkDescriptorSet m_descriptorSet;
	VkSampler m_sampler;
	VulkanImage m_vulkanImage;
	VkImageView m_imageView;

};
}
