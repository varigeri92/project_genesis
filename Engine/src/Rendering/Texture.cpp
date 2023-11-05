#include "Texture.h"
#include <array>
#include <vulkan/vulkan.h>
#include "../../API/Log.h"
#include "PipelineBuilder.h"
#include "stb_image.h"

namespace gns::rendering
{
	Texture::Texture(std::string path)
	{
		guid = core::Guid::GetNewGuid();
		//std::string path = "D:\\GenesisEngine\\Engine\\Assets\\Textures\\viking_room.png";
		int texWidth, texHeight, texChannels;
		m_pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		m_imageSize = texWidth * texHeight * 4;

		if (!m_pixels) {
			LOG_ERROR("Failed to load image: " << path.c_str());
			return;
		}
		width = texWidth;
		height = texHeight;
		channels = texChannels;

		format = VK_FORMAT_R8G8B8A8_SRGB;
		tiling = VK_IMAGE_TILING_OPTIMAL;
		usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	void Texture::CleanupTexture(Device* device)
	{
		vkDestroySampler(device->GetDevice(), m_sampler, nullptr);
		vkDestroyImageView(device->GetDevice(), m_imageView, nullptr);
		vkDestroyImage(device->GetDevice(), m_image, nullptr);
		vkFreeMemory(device->GetDevice(), m_imageMemory, nullptr);
	}
}
