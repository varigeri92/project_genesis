#pragma once
#include "../../AssetDatabase/AssetLoader.h"
#include "../Helpers/Buffer.h"

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
	Texture(std::string path);
	void* pixels;
	int width;
	int height;
	int chanels;
	void Create();
	void Dispose(Device* device) override;
	void Apply();
private:
	VkDescriptorSet descriptorSet;
	VkImage image;
	VmaAllocation allocation;
	VkImageView imageView;
	VkSampler m_sampler;
	Buffer stagingBuffer;
};
}
