#pragma once
#include "../../AssetDatabase/AssetLoader.h"
#include "../Helpers/Buffer.h"
#include <gnsAPI.h>
#include "VulkanImage.h"
namespace gns
{
	class Application;
}

namespace gns::rendering
{
class Texture : public  IDisposeable
{
	friend class Renderer;
	friend class gns::Application;
	
public:
	Texture() = default;
	GNS_API Texture(std::string path);
	void* pixels;
	int width;
	int height;
	int chanels;
	GNS_API void Create();
	GNS_API void Update(uint32_t binding);
	GNS_API void Dispose(Device* device) override;
	GNS_API void Apply();
	GNS_API void Use();

	VkDescriptorSet descriptorSet;
	VkImage image;
	VmaAllocation allocation;
	VkImageView imageView;
	VkSampler m_sampler;
	Buffer stagingBuffer;
private:
};
}
