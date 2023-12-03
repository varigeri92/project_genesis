#include "Texture.h"
#include <vulkan/vulkan.h>

#include "vklog.h"
#include "../Device.h"
#include "../Helpers/VkInitializer.h"


namespace gns::rendering
{
	Texture::Texture(std::string path)
	{
        AssetLoader::LoadTextureData(path, this);
	}

	void Texture::Create(Device* device)
	{
	    VkDeviceSize imageSize = width * height * 4;
	    VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	    stagingBuffer = CreateBuffer(device->m_allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		void* data;
	    vmaMapMemory(device->m_allocator, stagingBuffer._allocation, &data);
	    memcpy(data, pixels, static_cast<size_t>(imageSize));
	    vmaUnmapMemory(device->m_allocator, stagingBuffer._allocation);
	    free(pixels);

	    VkExtent3D imageExtent;
	    imageExtent.width = static_cast<uint32_t>(width);
	    imageExtent.height = static_cast<uint32_t>(height);
	    imageExtent.depth = 1;

	    VkImageCreateInfo dimg_info = ImageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

	    VmaAllocationCreateInfo dimg_allocinfo = {};
	    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	    vmaCreateImage(device->m_allocator, &dimg_info, &dimg_allocinfo, &image, &allocation, nullptr);
		/*
		 VkImageViewCreateInfo imageinfo = ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, image, VK_IMAGE_ASPECT_COLOR_BIT);
	    vkCreateImageView(device->m_device, &imageinfo, nullptr, &imageView);

		const VkSamplerCreateInfo samplerInfo = sampler_create_info(VK_FILTER_NEAREST);
		vkCreateSampler(device->m_device, &samplerInfo, nullptr, &m_sampler);

		//allocate the descriptor set for single-texture to use on the material
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device->m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &device->m_singleTextureSetLayout;

		vkAllocateDescriptorSets(device->m_device, &allocInfo, &descriptorSet);

		//write to the descriptor set so that it points to our empire_diffuse texture
		VkDescriptorImageInfo imageBufferInfo;
		imageBufferInfo.sampler = m_sampler;
		imageBufferInfo.imageView = imageView;
		imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet texture1 = write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSet, &imageBufferInfo, 0);

		vkUpdateDescriptorSets(device->m_device, 1, &texture1, 0, nullptr);
		 */
	}
	void Texture::Dispose(Device* device)
	{
	    vmaDestroyImage(device->m_allocator, image, allocation);
		vkDestroyImageView(device->m_device, imageView, nullptr);
		vkDestroySampler(device->m_device, m_sampler, nullptr);
	}

	void Texture::Apply(Device* device)
	{
		device->ImmediateSubmit(([&](VkCommandBuffer cmd) {
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer = {};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = image;
			imageBarrier_toTransfer.subresourceRange = range;
			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr,
				1, &imageBarrier_toTransfer);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;

			VkExtent3D imageExtent;
			imageExtent.width = static_cast<uint32_t>(width);
			imageExtent.height = static_cast<uint32_t>(height);
			imageExtent.depth = 1;

			copyRegion.imageExtent = imageExtent;

			//copy the buffer into the image
			vkCmdCopyBufferToImage(cmd, stagingBuffer._buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//barrier the image into the shader readable layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr,
				1, &imageBarrier_toReadable);
			}));
		stagingBuffer.Dispose(device);
	}
}
