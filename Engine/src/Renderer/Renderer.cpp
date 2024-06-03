#include "gnspch.h"
#include "Renderer.h"

#include "MeshData.h"
#include "PipelineBuilder.h"
#include "RenderSystem.h"
#include "Utils/VkHelpers.h"
#include "Utils/PipelineHelper.h"
#include "Shader.h"
#include "Utils/VkDebug.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "Material.h"
#include "../AssetDatabase/AssetLoader.h"
#include "../Window/Window.h"

namespace gns::rendering
{
		
	Renderer::Renderer(Device* device):m_device(device){}

	Renderer::~Renderer()
	{
		vkDeviceWaitIdle(m_device->m_device);
		delete(m_device);
	}

	void Renderer::UploadMesh(MeshData* mesh)
	{
		CreateVertexBuffers(mesh);
		CreateIndexBuffers(mesh);
		
	}
	void Renderer::CreatePipeline(Shader* shader)
	{
		VkShaderModule vertShader = CreateShaderModule(AssetLoader::LoadShader(shader->m_vertexShaderPath));
		VkShaderModule fragShader = CreateShaderModule(AssetLoader::LoadShader(shader->m_fragmentShaderPath));

		//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the m_shader modules per stage
		PipelineBuilder pipelineBuilder(m_device);
		pipelineBuilder._shaderStages.push_back(
			PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader));
		pipelineBuilder._shaderStages.push_back(
			PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader));

		std::vector<VkDescriptorSetLayout> setLayouts = { m_device->m_globalSetLayout };
		CreateSetLayoutBindings(shader);
		setLayouts.push_back(shader->shaderSetLayout);

		VkExtent2D scissor = { m_device->m_offscreenPass.width, m_device->m_offscreenPass.height };
		pipelineBuilder._vertexInputInfo = VertexInputStateCreateInfo();
		pipelineBuilder._inputAssembly = InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder._viewport.x = 0.0f;
		pipelineBuilder._viewport.y = 0.0f;
		pipelineBuilder._viewport.width = static_cast<float>(m_device->m_offscreenPass.width);
		pipelineBuilder._viewport.height = static_cast<float>(m_device->m_offscreenPass.width);
		pipelineBuilder._viewport.minDepth = 0.0f;
		pipelineBuilder._viewport.maxDepth = 1.0f;
		pipelineBuilder._scissor.offset = { 0, 0 };
		pipelineBuilder._scissor.extent = scissor;
		pipelineBuilder._rasterizer = RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
		pipelineBuilder._multisampling = MultisamplingStateCreateInfo();
		pipelineBuilder._colorBlendAttachment = ColorBlendAttachmentState();
		pipelineBuilder._depthStencil = DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

		VertexInputDescription vertexDescription = Vertex::GetVertexDescription();
		pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
		pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

		pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
		pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(MeshPushConstants);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


		VkPipelineLayoutCreateInfo layoutInfo = PipelineLayoutCreateInfo();
		layoutInfo.pPushConstantRanges = &push_constant;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		layoutInfo.pSetLayouts = setLayouts.data();

		_VK_CHECK(vkCreatePipelineLayout(m_device->m_device, &layoutInfo, nullptr, &shader->pipelineLayout),
			"Failed To create pipeline Layout");

		pipelineBuilder._pipelineLayout = shader->pipelineLayout;

		shader->pipeline = pipelineBuilder.Build(m_device->m_device, m_device->m_offscreenPass.renderPass);

		vkDestroyShaderModule(m_device->m_device, vertShader, nullptr);
		vkDestroyShaderModule(m_device->m_device, fragShader, nullptr);
	}


	bool Renderer::BeginFrame(uint32_t& swapchainImageIndex)
	{

		m_frameNumber++;
		//wait until the GPU has finished rendering the last frame. Timeout of 1 second
		_VK_CHECK(vkWaitForFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence, true, 1000000000), "--");
		_VK_CHECK(vkResetFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence), "--");


		//request image from the swapchain, one second timeout
		VkResult result = vkAcquireNextImageKHR(m_device->m_device, m_device->m_swapchain, 1000000000, m_device->GetCurrentFrame()._presentSemaphore, nullptr, &swapchainImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			LOG_VK_WARNING("NEED TO RECREATE SWAPCHAIN!");
			int w = 0, h = 0;
			m_device->m_window->GetExtent(w, h);
			m_device->RebuildSwapchain(w,h);
			m_frameBufferResized = true;
			return false;
		}
		else
		{
			_VK_CHECK(result, "Failed to acquire the next swapchain image!");
		}


		//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
		_VK_CHECK(vkResetCommandBuffer(m_device->GetCurrentFrame()._mainCommandBuffer, 0), "Failed to reset command buffer!");
		//naming it m_currentCommandBuffer for shorter writing
		m_device->GetCurrentFrame()._mainCommandBuffer = m_device->GetCurrentFrame()._mainCommandBuffer;

		//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
		VkCommandBufferBeginInfo cmdBeginInfo = {};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;

		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		const VkViewport viewport = { 0, 0, (float)m_device->m_offscreenPass.width, (float)m_device->m_offscreenPass.height, 0, 1 };
		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = { m_device->m_offscreenPass.width , m_device->m_offscreenPass.height };

		_VK_CHECK(vkBeginCommandBuffer(m_device->GetCurrentFrame()._mainCommandBuffer, &cmdBeginInfo), "Cannot begin cmdBuffer");
		vkCmdSetViewport(m_device->GetCurrentFrame()._mainCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(m_device->GetCurrentFrame()._mainCommandBuffer, 0, 1, &scissor);
		return true;
	}

	void Renderer::BeginRenderPass(uint32_t& swapchainImageIndex, bool gui)
	{
		VkClearValue clearValue;
		clearValue.color = { { 0.5f, 0.7f, 0.9f, 1.0f } };
		VkClearValue depthClear;
		depthClear.depthStencil.depth = 1.f;

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;

		rpInfo.renderPass = m_device->m_screenPass.renderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = m_device->m_vkb_swapchain.extent;
		rpInfo.framebuffer = m_device->m_screenPass.frameBuffers[swapchainImageIndex];
		rpInfo.clearValueCount = 2;
		VkClearValue clearValues[] = { clearValue, depthClear };
		rpInfo.pClearValues = &clearValues[0];
		vkCmdBeginRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void Renderer::BeginTextureRenderPass(uint32_t& swapchainImageIndex)
	{
		VkClearValue clearValue;
		clearValue.color = { { 0.5f, 0.7f, 0.9f, 1.0f } };
		VkClearValue depthClear;
		depthClear.depthStencil.depth = 1.f;

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;

		rpInfo.renderPass = m_device->m_offscreenPass.renderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = {};
		rpInfo.renderArea.extent.width = m_device->m_offscreenPass.width;
		rpInfo.renderArea.extent.height = m_device->m_offscreenPass.height;
		rpInfo.framebuffer = m_device->m_offscreenPass.frameBuffers[swapchainImageIndex];
		rpInfo.clearValueCount = 2;
		VkClearValue clearValues[] = { clearValue, depthClear };
		rpInfo.pClearValues = &clearValues[0];
		vkCmdBeginRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void Renderer::BeginGuiRenderPass(uint32_t& swapchainImageIndex)
	{
		VkClearValue clearValue;
		clearValue.color = { { 0.5f, 0.7f, 0.9f, 1.0f } };
		VkClearValue depthClear;
		depthClear.depthStencil.depth = 1.f;

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;

		rpInfo.renderPass = m_device->m_screenPass.renderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = m_device->m_vkb_swapchain.extent;
		rpInfo.framebuffer = m_device->m_screenPass.frameBuffers[swapchainImageIndex];
		rpInfo.clearValueCount = 2;
		VkClearValue clearValues[] = { clearValue, depthClear };
		rpInfo.pClearValues = &clearValues[0];
		vkCmdBeginRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void Renderer::EndRenderPass(uint32_t& swapchainImageIndex)
	{
		vkCmdEndRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer);
	}

	void Renderer::Draw(MeshData* mesh, Material* material, int index, size_t uniformSize)
	{
		if (m_currentMaterial != material || m_pipelineBound == false)
		{
			m_currentMaterial = material;
			vkCmdBindPipeline(m_device->GetCurrentFrame()._mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipeline);

			uint32_t uniform_offset = m_device->PadUniformBufferSize(uniformSize) * m_device->m_imageIndex;

			vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipelineLayout, 0, 1,
				&m_device->GetCurrentFrame()._globalDescriptor, 1, &uniform_offset);


			vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipelineLayout, 1, 1, &material->descriptorSet,
				0, nullptr);

			/*
			vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 1, 1,
				&m_device->GetCurrentFrame()._objectDescriptor, 0, nullptr);
			*/
			m_pipelineBound = true;
		}

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(m_device->GetCurrentFrame()._mainCommandBuffer,
			0, 1, &mesh->_vertexBuffer._buffer, &offset);
		vkCmdBindIndexBuffer(m_device->GetCurrentFrame()._mainCommandBuffer,
			mesh->_indexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_device->GetCurrentFrame()._mainCommandBuffer, static_cast<uint32_t>(mesh->_indices.size()),
			1, 0, 0, 0);
	}

	void Renderer::EndFrame(uint32_t& swapchainImageIndex)
	{
		//finalize the command buffer (we can no longer add commands, but it can now be executed)
		_VK_CHECK(vkEndCommandBuffer(m_device->GetCurrentFrame()._mainCommandBuffer), "failed to end m_currentCommandBuffer buffer!");
		//prepare the submission to the queue.
		//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		//we will signal the _renderSemaphore, to signal that rendering has finished

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &m_device->GetCurrentFrame()._presentSemaphore;

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &m_device->GetCurrentFrame()._renderSemaphore;

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &m_device->GetCurrentFrame()._mainCommandBuffer;

		//submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		_VK_CHECK(vkQueueSubmit(m_device->m_graphicsQueue, 1, &submit, m_device->GetCurrentFrame()._renderFence), "XXX");

		// this will put the image we just rendered into the visible window.
		// we want to wait on the _renderSemaphore for that,
		// as it's necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		presentInfo.pSwapchains = &m_device->m_swapchain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &m_device->GetCurrentFrame()._renderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &swapchainImageIndex;

		VkResult result = vkQueuePresentKHR(m_device->m_graphicsQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized)
		{
			LOG_WARNING("NEED TO RECREATE SWAPCHAIN!");
			m_frameBufferResized = false;
			int w = 0, h = 0;
			m_device->m_window->GetExtent(w, h);
			m_device->RebuildSwapchain(w, h);
		}
		else
		{
			_VK_CHECK(result, "Failed to present image! on");
		}
		m_pipelineBound = false;
	}

	VkShaderModule Renderer::CreateShaderModule(std::vector<uint32_t> buffer)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;

		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_device->m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}
		return shaderModule;
	}

	void Renderer::CreateVertexBuffers(MeshData* mesh)
	{
		const size_t bufferSize = mesh->_vertices.size() * sizeof(Vertex);
		//allocate staging buffer
		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.pNext = nullptr;

		stagingBufferInfo.size = bufferSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		//let the VMA library know that this data should be on CPU RAM
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		Buffer stagingBuffer;

		//allocate the buffer
		_VK_CHECK(vmaCreateBuffer(m_device->m_allocator, &stagingBufferInfo, &vmaallocInfo,
			&stagingBuffer._buffer,
			&stagingBuffer._allocation,
			nullptr), "Failed to create Buffer");

		void* data;
		vmaMapMemory(m_device->m_allocator, stagingBuffer._allocation, &data);
		memcpy(data, mesh->_vertices.data(), mesh->_vertices.size() * sizeof(Vertex));
		vmaUnmapMemory(m_device->m_allocator, stagingBuffer._allocation);


		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.pNext = nullptr;
		//this is the total size, in bytes, of the buffer we are allocating
		vertexBufferInfo.size = bufferSize;
		//this buffer is going to be used as a Vertex Buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		//let the VMA library know that this data should be GPU native
		vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//allocate the buffer
		_VK_CHECK(vmaCreateBuffer(m_device->m_allocator, &vertexBufferInfo, &vmaallocInfo,
			&mesh->_vertexBuffer._buffer,
			&mesh->_vertexBuffer._allocation,
			nullptr), "");

		m_device->ImmediateSubmit([&mesh, stagingBuffer, bufferSize](VkCommandBuffer cmd)
			{
				VkBufferCopy copy;
				copy.dstOffset = 0;
				copy.srcOffset = 0;
				copy.size = bufferSize;
				vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh->_vertexBuffer._buffer, 1, &copy);
			});
		m_device->DisposeBuffer(stagingBuffer);
	}

	void Renderer::CreateIndexBuffers(MeshData* mesh)
	{
		size_t size = sizeof(mesh->_indices[0]);
		const size_t bufferSize = mesh->_indices.size() * size;
		//allocate staging buffer
		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.pNext = nullptr;

		stagingBufferInfo.size = bufferSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		//let the VMA library know that this data should be on CPU RAM
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		Buffer stagingBuffer;

		//allocate the buffer
		_VK_CHECK(vmaCreateBuffer(m_device->m_allocator, &stagingBufferInfo, &vmaallocInfo,
			&stagingBuffer._buffer,
			&stagingBuffer._allocation,
			nullptr), "Failed to create Buffer");

		void* data;
		vmaMapMemory(m_device->m_allocator, stagingBuffer._allocation, &data);
		memcpy(data, mesh->_indices.data(), mesh->_indices.size() * size);
		vmaUnmapMemory(m_device->m_allocator, stagingBuffer._allocation);


		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.pNext = nullptr;
		//this is the total size, in bytes, of the buffer we are allocating
		indexBufferInfo.size = bufferSize;
		//this buffer is going to be used as a Vertex Buffer
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		//let the VMA library know that this data should be GPU native
		vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//allocate the buffer
		_VK_CHECK(vmaCreateBuffer(m_device->m_allocator, &indexBufferInfo, &vmaallocInfo,
			&mesh->_indexBuffer._buffer,
			&mesh->_indexBuffer._allocation,
			nullptr), "");

		m_device->ImmediateSubmit([&mesh, stagingBuffer, bufferSize](VkCommandBuffer cmd)
			{
				VkBufferCopy copy;
				copy.dstOffset = 0;
				copy.srcOffset = 0;
				copy.size = bufferSize;
				vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh->_indexBuffer._buffer, 1, &copy);
			});
		m_device->DisposeBuffer(stagingBuffer);
	}

	void Renderer::CreateSetLayoutBindings(Shader* shader)
	{
		std::vector<VkDescriptorSetLayoutBinding> textureBindings =
		{
			DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
			DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
			DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
			DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
		};
		m_device->CreateDescriptorSetLayout(&shader->shaderSetLayout, textureBindings.data(), textureBindings.size());
	}


	void Renderer::UpdatePushConstant(glm::mat4 modelMatrix, Material* material)
	{
		MeshPushConstants push_constants = {};
		push_constants.model_matrix = modelMatrix;
		push_constants.camera_matrix = glm::mat4(1);
		vkCmdPushConstants(m_device->GetCurrentFrame()._mainCommandBuffer, material->m_shader->pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &push_constants);
	}

	void Renderer::UpdateGlobalUbo(void* src_bufferData, size_t size)
	{
		void* data;
		vmaMapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._cameraBuffer._allocation, &data);
		memcpy(data, src_bufferData, size);
		vmaUnmapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._cameraBuffer._allocation);
	}

	void Renderer::UpdateSceneDataUbo(const void* data, size_t size)
	{
		int frameIndex = m_frameNumber % m_device->m_imageCount;
		char* sceneData;
		vmaMapMemory(m_device->m_allocator, m_device->m_sceneParameterBuffer._allocation, (void**)&sceneData);
		sceneData += m_device->PadUniformBufferSize(size) * frameIndex;
		memcpy(sceneData, data, size);
		vmaUnmapMemory(m_device->m_allocator, m_device->m_sceneParameterBuffer._allocation);
	}

	void Renderer::UpdateObjectData(void* src_data, size_t size)
	{
		void* objectData;
		vmaMapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._objectBuffer._allocation, &objectData);
		memcpy(objectData, src_data, size);
		vmaUnmapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._objectBuffer._allocation);
	}

	void Renderer::UpdateMaterialUniformBuffer(void* src_data, size_t size, const std::shared_ptr<Material>& material)
	{
		/* 
		void* objectData;
		vmaMapMemory(m_device->m_allocator, material->m_uniformBuffer._allocation, &objectData);
		memcpy(objectData, src_data, size);
		vmaUnmapMemory(m_device->m_allocator, material->m_uniformBuffer._allocation);
		*/
	}
}
