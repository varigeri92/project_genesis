#include "Renderer.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "Log.h"
#include "BufferHelper.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../Window/Window.h"
#include "../mesh.h"
#include "../Core/Time.h"
#include "Texture.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "VulkanImage.h"
#include "Material.h"

namespace gns::rendering
{
	Renderer::Renderer(Window* window) : m_window{window}
	{
		m_device = new Device(m_window);
		m_pipelineBuilder = new PipelineBuilder(m_device);
	}

	void Renderer::CreateRenderer(Material& material)
	{
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSyncObjects();
		//========================================//
		m_pipelineBuilder->BuildPipeline(material);

		CreateFranmeBuffers();
	}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_device->GetDevice());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_device->GetDevice(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device->GetDevice(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device->GetDevice(), m_inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_device->GetDevice(), m_commandPool, nullptr);
		for (auto framebuffer : m_frameBuffers) {
			vkDestroyFramebuffer(m_device->GetDevice(), framebuffer, nullptr);
		}
		delete(m_pipelineBuilder);
		delete(m_device);
	}

	void Renderer::CreateVertexBuffer(Mesh* mesh)
	{
		VkDeviceSize bufferSize = sizeof(mesh->Vertices[0]) * mesh->Vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		BufferHelper::CreateBuffer(m_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, mesh->Vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device->GetDevice(), stagingBufferMemory);

		BufferHelper::CreateBuffer(m_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			mesh->m_vertexBuffer, mesh->m_vertexBufferMemory);

		BufferHelper::CopyBuffer(m_device, stagingBuffer, mesh->m_vertexBuffer, bufferSize, m_commandPool);

		vkDestroyBuffer(m_device->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->GetDevice(), stagingBufferMemory, nullptr);
	}

	void Renderer::CreateIndexBuffer(Mesh* mesh)
	{
		VkDeviceSize bufferSize = sizeof(mesh->Indices[0]) * mesh->Indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		BufferHelper::CreateBuffer(m_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, mesh->Indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device->GetDevice(), stagingBufferMemory);

		BufferHelper::CreateBuffer(m_device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mesh->m_indexBuffer, mesh->m_indexBufferMemory);

		BufferHelper::CopyBuffer(m_device, stagingBuffer, mesh->m_indexBuffer, bufferSize, m_commandPool);

		vkDestroyBuffer(m_device->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->GetDevice(), stagingBufferMemory, nullptr);
	}

	float rotate = 0;
	void Renderer::UpdateUniformBuffer(uint32_t currentImage, UniformBufferObject& ubo)
	{
		memcpy(m_pipelineBuilder->m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}
	void Renderer::DeleteMesh(Mesh* mesh)
	{
		vkDeviceWaitIdle(m_device->GetDevice());
		vkDestroyBuffer(m_device->GetDevice(), mesh->m_vertexBuffer, nullptr);
		vkFreeMemory(m_device->GetDevice(), mesh->m_vertexBufferMemory, nullptr);
		vkDestroyBuffer(m_device->GetDevice(), mesh->m_indexBuffer, nullptr);
		vkFreeMemory(m_device->GetDevice(), mesh->m_indexBufferMemory, nullptr);
	}

	void Renderer::DeleteTexture(Texture* texture)
	{
		texture->CleanupTexture(m_device);
	}

	void Renderer::DrawFrame()
	{
		uint32_t imageIndex = 0;
		if(!BeginFrame(imageIndex))
			return;

		EndFrame(imageIndex);
	}

	void Renderer::DrawMesh(Mesh* mesh, uint32_t imageIndex, Material& material)
	{
		vkCmdBindPipeline(CommandBuffers.drawBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipeline());

		VkBuffer vertexBuffers[] = { mesh->m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffers.drawBuffers[m_currentFrame], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(CommandBuffers.drawBuffers[m_currentFrame], mesh->m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(CommandBuffers.drawBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipelineLayout(),
			0, 1, &material.texture->m_descriptorSets[m_currentFrame], 0, nullptr);

		vkCmdDrawIndexed(CommandBuffers.drawBuffers[m_currentFrame], static_cast<uint32_t>(mesh->Indices.size()),
			1, 0, 0, 0);
	}

	bool Renderer::BeginFrame(uint32_t& imageIndex)
	{
		if (m_frameBufferMinimized == true) return false;

		vkWaitForFences(m_device->GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
		

		VkResult result = vkAcquireNextImageKHR(m_device->GetDevice(), m_device->m_swapChain, UINT64_MAX,
			m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
		/**/
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(m_device->GetDevice(), 1, &m_inFlightFences[m_currentFrame]);
		vkResetCommandBuffer(CommandBuffers.drawBuffers[m_currentFrame], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional
		_VK_CHECK(vkBeginCommandBuffer(CommandBuffers.drawBuffers[m_currentFrame], &beginInfo), "failed to begin recording command buffer!");

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_device->GetSwapchainExtent().width);
		viewport.height = static_cast<float>(m_device->GetSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(CommandBuffers.drawBuffers[m_currentFrame], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_device->GetSwapchainExtent();
		vkCmdSetScissor(CommandBuffers.drawBuffers[m_currentFrame], 0, 1, &scissor);

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_pipelineBuilder->GetRenderPass();
		renderPassInfo.framebuffer = m_frameBuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_device->GetSwapchainExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(CommandBuffers.drawBuffers[m_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		return true;
	}

	void Renderer::BeginDraw(Material* material)
	{
		if (!m_isPipelineBound)
			vkCmdBindPipeline(CommandBuffers.drawBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipeline());
		
		vkCmdBindDescriptorSets(CommandBuffers.drawBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipelineLayout(),
			0, 1, &material->texture->m_descriptorSets[m_currentFrame], 0, nullptr);

		m_isPipelineBound = true;
	}

	void Renderer::Submit(Mesh* mesh)
	{

		VkBuffer vertexBuffers[] = { mesh->m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffers.drawBuffers[m_currentFrame], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(CommandBuffers.drawBuffers[m_currentFrame], mesh->m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CommandBuffers.drawBuffers[m_currentFrame], static_cast<uint32_t>(mesh->Indices.size()),
			1, 0, 0, 0);
	}

	void Renderer::EndDraw(uint32_t imageIndex)
	{
	}

	void Renderer::EndFrame(uint32_t& imageIndex)
	{
		vkCmdEndRenderPass(CommandBuffers.drawBuffers[m_currentFrame]);
		_VK_CHECK(vkEndCommandBuffer(CommandBuffers.drawBuffers[m_currentFrame]), "failed to End command buffer!")
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffers.drawBuffers[m_currentFrame];

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		_VK_CHECK(vkQueueSubmit(m_device->m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]),
			"failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { m_device->m_swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		VkResult result = vkQueuePresentKHR(m_device->m_presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		m_isPipelineBound = false;
	}

	void Renderer::CreateFranmeBuffers()
	{
		m_frameBuffers.resize(m_device->m_swapChainImageViews.size());
		int width, height = 0;
		m_window->GetExtent(width, height);
		for (size_t i = 0; i < m_device->m_swapChainImageViews.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				m_device->m_swapChainImageViews[i],
				m_pipelineBuilder->m_depthImageView
			};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_pipelineBuilder->GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			_VK_CHECK(vkCreateFramebuffer(m_device->GetDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]),
				"failed to create framebuffer!");
		}
	}

	void Renderer::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_device->m_indices.graphicsFamily.value();
		_VK_CHECK(vkCreateCommandPool(m_device->GetDevice(), &poolInfo, nullptr, &m_commandPool),
			"failed to create command pool!");
	}

	void Renderer::CreateCommandBuffers()
	{
		CommandBuffers.drawBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.drawBuffers.size());

		_VK_CHECK(vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, CommandBuffers.drawBuffers.data()),
			"failed to allocate command buffers!")

		CommandBuffers.compositeBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.compositeBuffers.size());

		_VK_CHECK(vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, CommandBuffers.compositeBuffers.data()),
			"failed to allocate command buffers!")
	}

	void Renderer::RecordCommandBuffer(VkCommandBuffer commandbuffer, uint32_t imageIndex, Mesh* mesh, Material& material)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		_VK_CHECK(vkBeginCommandBuffer(commandbuffer, &beginInfo), "failed to begin recording command buffer!")

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_pipelineBuilder->GetRenderPass();
		renderPassInfo.framebuffer = m_frameBuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_device->GetSwapchainExtent();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();


		vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_device->GetSwapchainExtent().width);
		viewport.height = static_cast<float>(m_device->GetSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandbuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_device->GetSwapchainExtent();
		vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[] = { mesh->m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandbuffer, mesh->m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineBuilder->GetPipelineLayout(),
			0, 1, &material.texture->m_descriptorSets[m_currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandbuffer, static_cast<uint32_t>(mesh->Indices.size()), 
			1, 0, 0, 0);

		vkCmdEndRenderPass(commandbuffer);
		_VK_CHECK(vkEndCommandBuffer(commandbuffer),"failed to End command buffer!")

	}

	void Renderer::CreateSyncObjects()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			_VK_CHECK(vkCreateSemaphore(m_device->GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]),
				"Cannot create Image available Semaphore!");
				_VK_CHECK(vkCreateSemaphore(m_device->GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]),
					"Cannot create Render finished Semaphore!");
			_VK_CHECK(vkCreateFence(m_device->GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]), 
				"Cannot create In flight fences!");
		}
	}

	void Renderer::RecreateSwapChain()
	{
		LOG_INFO("RECREATE SWAPCHAIN!");
		vkDeviceWaitIdle(m_device->GetDevice());
		CleanupSwapChain();
		m_device->RecreateSwapChain(m_window);
		m_pipelineBuilder->CleanupDepthResources();
		m_pipelineBuilder->CreateDepthResources();
		CreateFranmeBuffers();
		m_frameBufferResized = false;
	}

	void Renderer::CleanupSwapChain()
	{
		for (size_t i = 0; i < m_frameBuffers.size(); i++) {
			vkDestroyFramebuffer(m_device->GetDevice(), m_frameBuffers[i], nullptr);
		}

		for (size_t i = 0; i < m_device->m_swapChainImageViews.size(); i++) {
			vkDestroyImageView(m_device->GetDevice(), m_device->m_swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(m_device->GetDevice(), m_device->m_swapChain, nullptr);
	}

	void Renderer::CreateTexture(Texture& texture)
	{
		CreateTexture_Internal(texture);
		CreateTextureImageView(texture);
		CreateTextureSampler(texture);
		m_pipelineBuilder->CreateDescriptorSets(texture.m_imageView, texture.m_sampler, texture);
	}


	void Renderer::CreateTexture_Internal(Texture& texture)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		BufferHelper::CreateBuffer(m_device, texture.m_imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device->GetDevice(), stagingBufferMemory, 0, texture.m_imageSize, 0, &data);
		memcpy(data, (stbi_uc*)texture.m_pixels, static_cast<size_t>(texture.m_imageSize));
		vkUnmapMemory(m_device->GetDevice(), stagingBufferMemory);

		stbi_image_free(texture.m_pixels);
		VulkanImage::CreateImage(m_device, texture.width, texture.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texture.m_image, texture.m_imageMemory);
		TransitionImageLayout(texture.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, texture.m_image, static_cast<uint32_t>(texture.width), static_cast<uint32_t>(texture.height));
		TransitionImageLayout(texture.m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(m_device->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_device->GetDevice(), stagingBufferMemory, nullptr);
	}

	void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
	                                     VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO


		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		VkCommandBuffer commandBuffer = BufferHelper::BeginSingleTimeCommands(m_device, m_commandPool);
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,0,
			0, nullptr,0, nullptr,1, &barrier);
		BufferHelper::EndSingleTimeCommands(m_device, commandBuffer, m_commandPool);
	}

	void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = BufferHelper::BeginSingleTimeCommands(m_device, m_commandPool);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer,buffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &region);

		BufferHelper::EndSingleTimeCommands(m_device, commandBuffer, m_commandPool);
	}

	void Renderer::CreateTextureImageView(Texture& texture)
	{
		texture.m_imageView = VulkanImage::CreateImageView(m_device, texture.m_image, VK_FORMAT_R8G8B8A8_SRGB);
	}

	void Renderer::CreateTextureSampler(Texture& texture)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.f; //m_device->m_deviceProperties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_device->GetDevice(), &samplerInfo, nullptr, &texture.m_sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
}
