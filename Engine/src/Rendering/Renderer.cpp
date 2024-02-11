#include "Renderer.h"
#include "Log.h"
#include "rendererInclude.h"
#include "RenderSystem.h"
#include "vklog.h"
#include "../AssetDatabase/AssetLoader.h"
#include "../Input.h"
#include "SDL2/SDL_keycode.h"
#include "../Core/Time.h"
#include "DataObjects/Texture.h"
#include "Helpers/VkInitializer.h"
#include "ImGui/imgui_impl_vulkan.h"

namespace gns::rendering
{
Renderer::Renderer(Window* window, size_t buffersize)
{
	LOG_VK_WARNING("Create Renderer");
	m_device = new Device(window);
	LOG_VK_WARNING("Created Device");
	m_device->InitDescriptors(buffersize);
	LOG_VK_WARNING("Descriuptors initialized!");
	RenderSystem::S_Device = m_device;
	LOG_VK_WARNING("Device Assigned");
}

Renderer::~Renderer()
{
	vkDeviceWaitIdle(m_device->m_device);
	for (size_t i =0; i<_disposeQueue.size(); i++)
	{
		_disposeQueue[i]->Dispose(m_device);
	}
	delete(m_device);
}

Renderer::Renderer(Renderer& other)
{
	LOG_INFO("Hello copy!");
}

void Renderer::UploadMesh(Mesh* mesh)
{
	CreateVertexBuffers(mesh);
	CreateIndexBuffers(mesh);
}

void Renderer::CreatePipelineForMaterial(std::shared_ptr<Shader> shader)
{
	VkShaderModule vertShader = CreateShaderModule(AssetLoader::LoadShader(shader->vertexShaderPath));
	VkShaderModule fragShader = CreateShaderModule(AssetLoader::LoadShader(shader->fragmentShaderPath));

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the m_shader modules per stage
	PipelineBuilder pipelineBuilder(m_device);
	pipelineBuilder._shaderStages.push_back(
		PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader));
	pipelineBuilder._shaderStages.push_back(
		PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader));

	std::vector<VkDescriptorSetLayout> setLayouts = {
		m_device->m_globalSetLayout,
		m_device->m_singleTextureSetLayout
	};

	//setLayouts.push_back(m_device->m_singleTextureSetLayout);
	pipelineBuilder._vertexInputInfo = VertexInputStateCreateInfo();
	pipelineBuilder._inputAssembly = InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = static_cast<float>(m_device->m_vkb_swapchain.extent.width);
	pipelineBuilder._viewport.height = static_cast<float>(m_device->m_vkb_swapchain.extent.height);
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;
	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = m_device->m_vkb_swapchain.extent;
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

	shader->pipeline = pipelineBuilder.Build(m_device->m_device, m_device->m_renderPass);

	vkDestroyShaderModule(m_device->m_device, vertShader, nullptr);
	vkDestroyShaderModule(m_device->m_device, fragShader, nullptr);
}

void Renderer::DisposeObject(std::shared_ptr<IDisposeable> object)
{
	_disposeQueue.push_back(object);
}

bool Renderer::BeginFrame(uint32_t& swapchainImageIndex)
{
	//request image from the swapchain, one second timeout
	VkResult result =vkAcquireNextImageKHR(m_device->m_device, m_device->m_swapchain, 1000000000, m_device->GetCurrentFrame()._presentSemaphore,nullptr, &swapchainImageIndex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		LOG_VK_WARNING("NEED TO RECREATE SWAPCHAIN!");
		m_device->RebuildSwapchain();
		m_framebufferResized = true;
		return false;
	}
	else
	{
		_VK_CHECK(result, "Failed to acquire the next swapchain image!");
	}

	m_frameNumber++;
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	_VK_CHECK(vkWaitForFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence, true, 1000000000), "--");
	_VK_CHECK(vkResetFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence), "--");

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

	VkViewport viewport = { 0, 0, (float)m_device->m_vkb_swapchain.extent.width, (float)m_device->m_vkb_swapchain.extent.height, 0, 1 };
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = m_device->m_vkb_swapchain.extent;

	_VK_CHECK(vkBeginCommandBuffer(m_device->GetCurrentFrame()._mainCommandBuffer, &cmdBeginInfo), "Cannot begin cmdBuffer");
	vkCmdSetViewport(m_device->GetCurrentFrame()._mainCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(m_device->GetCurrentFrame()._mainCommandBuffer, 0, 1, &scissor);
	return true;
}

void Renderer::BeginRenderPass(uint32_t& swapchainImageIndex, bool gui)
{
	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;
	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us

	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = gui? m_device->m_guiPass : m_device->m_renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = m_device->m_vkb_swapchain.extent;
	rpInfo.framebuffer = m_device->m_frameBuffers[swapchainImageIndex];
	rpInfo.clearValueCount = 2;
	VkClearValue clearValues[] = { clearValue, depthClear };
	rpInfo.pClearValues = &clearValues[0];
	vkCmdBeginRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::EndRenderPass(uint32_t& swapchainImageIndex)
{
	//finalize the render pass
	vkCmdEndRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer);
}

void Renderer::Draw(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, int index, size_t uniformSize)
{
	if(m_material_ptr != material || m_pipelineBound == false)
	{
		m_material_ptr = material;
		vkCmdBindPipeline(m_device->GetCurrentFrame()._mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipeline);

		uint32_t uniform_offset = m_device->PadUniformBufferSize(uniformSize) * m_device->m_imageIndex;

		vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipelineLayout, 0, 1, 
			&m_device->GetCurrentFrame()._globalDescriptor, 1, &uniform_offset);


		if(material->m_texture != nullptr)
			vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer, 
				VK_PIPELINE_BIND_POINT_GRAPHICS, material->m_shader->pipelineLayout, 1, 1, &material->m_texture->descriptorSet,
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
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		LOG_WARNING("NEED TO RECREATE SWAPCHAIN!");
		m_framebufferResized = false;
		m_device->RebuildSwapchain();
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

void Renderer::CreateVertexBuffers(Mesh* mesh)
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
	vmaDestroyBuffer(m_device->m_allocator, stagingBuffer._buffer, stagingBuffer._allocation);
}

void Renderer::CreateIndexBuffers(Mesh* mesh)
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
	vmaDestroyBuffer(m_device->m_allocator, stagingBuffer._buffer, stagingBuffer._allocation);
}


void Renderer::UpdatePushConstant( glm::mat4 modelMatrix, std::shared_ptr<Material> material)
{
	MeshPushConstants push_constants = {};
	glm::vec3 camPos = { 0.f,-6.f,-20.f };
	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 1000.0f);
	projection[1][1] *= -1;

	glm::mat4 matrix = projection * view;

	push_constants.model_matrix = modelMatrix;
	push_constants.camera_matrix = matrix;
	vkCmdPushConstants(m_device->GetCurrentFrame()._mainCommandBuffer, material->m_shader->pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &push_constants);
}

void Renderer::UpdateGlobalUbo(GPUCameraData src_bufferData)
{
	void* data;
	vmaMapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._cameraBuffer._allocation, &data);
	memcpy(data, &src_bufferData, sizeof(GPUCameraData));
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

void Renderer::UpdateMaterialUniformBuffer(void* src_data, size_t size, Material* material)
{
	void* objectData;
	vmaMapMemory(m_device->m_allocator, material->m_shader->materialUniformBuffer._allocation, &objectData);
	memcpy(objectData, src_data, size);
	vmaUnmapMemory(m_device->m_allocator, material->m_shader->materialUniformBuffer._allocation);
}

void Renderer::CreateTextureResources(std::shared_ptr<Texture> texture)
{
	VkImageViewCreateInfo imageinfo = ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, texture->image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(m_device->m_device, &imageinfo, nullptr, &texture->imageView);

	const VkSamplerCreateInfo samplerInfo = sampler_create_info(VK_FILTER_NEAREST);
	vkCreateSampler(m_device->m_device, &samplerInfo, nullptr, &texture->m_sampler);

	//allocate the descriptor set for single-m_texture to use on the material
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_device->m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_device->m_singleTextureSetLayout;
	vkAllocateDescriptorSets(m_device->m_device, &allocInfo, &texture->descriptorSet);

	//write to the descriptor set so that it points to our empire_diffuse m_texture
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = texture->m_sampler;
	imageBufferInfo.imageView = texture->imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture->descriptorSet,
		&imageBufferInfo, 0);

	vkUpdateDescriptorSets(m_device->m_device, 1, &texture1, 0, nullptr);
}
}
