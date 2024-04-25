#pragma once
#include "Utils/Buffer.h"
#include "Utils/VulkanImage.h"

namespace gns::rendering
{
	class Texture
	{
	public:
		GNS_API Texture() = delete;
		GNS_API Texture(std::string path);
		static GNS_API std::shared_ptr<Texture> CreateTexture(std::string path);
		GNS_API ~Texture();
		void* pixels;
		int width;
		int height;
		int chanels;
		GNS_API void Create();
		GNS_API void Update(uint32_t binding);
		GNS_API void Apply();
		GNS_API void Destroy();

		VkDescriptorSet m_descriptorSet;
		VkSampler m_sampler;
		VulkanImage m_vulkanImage;
		VkImageView m_imageView;
		Buffer m_stagingBuffer;
	private:
		bool m_resourcesCleared;
	};
}
