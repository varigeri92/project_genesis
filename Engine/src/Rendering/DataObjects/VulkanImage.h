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
	
	class VulkanTexture : public  IDisposeable
	{
		friend class Renderer;
		friend class gns::Application;

	protected:
		VulkanTexture() = default;
		void Create();
		void Update(uint32_t binding);
		void Dispose(Device* device) override;
	private:

		Buffer stagingBuffer;

		VkDescriptorSet descriptorSet;
		VkImageView imageView;
		VkSampler m_sampler;

		VulkanImage vulkanImage;
	};
}
