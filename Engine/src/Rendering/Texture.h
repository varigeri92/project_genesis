#pragma once
#include <string>
#include "VulkanImage.h"
#include "../AssetDatabase/Guid.h"

namespace gns::rendering
{
	class Device;
	class PipelineBuilder;

	class Texture
	{
		friend class PipelineBuilder;
		friend class Renderer;
	public:
		Texture(std::string path);
		~Texture() = default;

		core::guid guid;

		uint32_t width;
		uint32_t height;
		uint32_t channels;
		VkFormat format;
		VkImageTiling tiling;
		VkMemoryPropertyFlags properties;
		VkImageUsageFlags usage;
	private:
		VkImage m_image;
		VkDeviceMemory m_imageMemory;
		VkImageView m_imageView;
		VkSampler m_sampler;
		std::vector<VkDescriptorSet> m_descriptorSets;
		void* m_pixels = nullptr;
		VkDeviceSize m_imageSize;
		void CleanupTexture(Device* device);
	};
}

