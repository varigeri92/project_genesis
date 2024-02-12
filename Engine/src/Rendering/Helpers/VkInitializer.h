#pragma once
#include <vulkan/vulkan.h>


VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

VkDescriptorSetLayoutBinding DescriptorsetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);

VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkSubmitInfo SubmitInfo(VkCommandBuffer* cmd);

VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

VkDescriptorBufferInfo CreateBufferInfo(VkBuffer buffer, VkDeviceSize range, VkDeviceSize offset = 0);

VkDescriptorSetAllocateInfo CreateAllocateInfo(VkDescriptorPool pool, const VkDescriptorSetLayout* setLayout, uint32_t setCount = 1);

VkDescriptorImageInfo CreateDescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);