#pragma once
#include "../../AssetDatabase/AssetLoader.h"
#include "../Helpers/Buffer.h"
#include <gnsAPI.h>

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
	GEN_API Texture(std::string path);
	void* pixels;
	int width;
	int height;
	int chanels;
	GEN_API void Create();
	GEN_API void Dispose(Device* device) override;
	GEN_API void Apply();
private:
	VkDescriptorSet descriptorSet;
	VkImage image;
	VmaAllocation allocation;
	VkImageView imageView;
	VkSampler m_sampler;
	Buffer stagingBuffer;
};
}
