#pragma once
#include "../../AssetDatabase/AssetLoader.h"
#include "../Helpers/Buffer.h"

namespace gns::rendering
{
class Texture : public  Disposeable
{
	friend class Renderer;
public:
	Texture() = default;
	Texture(std::string path);
	void* pixels;
	int width;
	int height;
	int chanels;
	void Create(Device* device);
	void Dispose(Device* device) override;
	void Apply(Device* device);
private:
	VkDescriptorSet descriptorSet;
	VkImage image;
	VmaAllocation allocation;
	VkImageView imageView;
	VkSampler m_sampler;
	Buffer stagingBuffer;
};
}
