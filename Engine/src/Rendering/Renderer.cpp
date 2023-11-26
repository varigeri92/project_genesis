#include "Renderer.h"
#include <glm/gtx/transform.hpp>
#include "Log.h"
#include "rendererInclude.h"
#include "vklog.h"
#include "../AssetDatabase/AssetLoader.h"
#include "../Input.h"
#include "SDL2/SDL_keycode.h"
#include "../Core/Time.h"
namespace gns::rendering
{
	int shader_selector = 0;
	int shaderCount = 2;
Renderer::Renderer(Window* window) 
{
	m_device = new Device(window);
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

void Renderer::UploadMesh(Mesh* mesh)
{
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//this is the total size, in bytes, of the buffer we are allocating
	bufferInfo.size = mesh->_vertices.size() * sizeof(Vertex);
	//this buffer is going to be used as a Vertex Buffer
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//allocate the buffer
	_VK_CHECK(vmaCreateBuffer(m_device->m_allocator, &bufferInfo, &vmaallocInfo,
		&mesh->_vertexBuffer._buffer,
		&mesh->_vertexBuffer._allocation,
		nullptr), "Buffer creation failed");

	void* data;
	vmaMapMemory(m_device->m_allocator, mesh->_vertexBuffer._allocation, &data);

	memcpy(data, mesh->_vertices.data(), mesh->_vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(m_device->m_allocator, mesh->_vertexBuffer._allocation);
}

void Renderer::CreatePipelineForMaterial(std::shared_ptr<Material> material)
{
	VkShaderModule vertShader = CreateShaderModule(AssetLoader::LoadShader(material->shader->vertexShaderPath));
	VkShaderModule fragShader = CreateShaderModule(AssetLoader::LoadShader(material->shader->fragmentShaderPath));

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder(m_device);
	pipelineBuilder._shaderStages.push_back(
		PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader));
	pipelineBuilder._shaderStages.push_back(
		PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader));

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

	std::vector<VkDescriptorSetLayout> setLayouts = {m_device->m_globalSetLayout };

	VkPipelineLayoutCreateInfo layoutInfo = PipelineLayoutCreateInfo();
	layoutInfo.pPushConstantRanges = &push_constant;
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	layoutInfo.pSetLayouts = setLayouts.data();

	_VK_CHECK(vkCreatePipelineLayout(m_device->m_device, &layoutInfo, nullptr, &material->pipelineLayout),
		"Failed To create pipeline Layout");

	pipelineBuilder._pipelineLayout = material->pipelineLayout;

	material->pipeline = pipelineBuilder.Build(m_device->m_device, m_device->m_renderPass);

	vkDestroyShaderModule(m_device->m_device, vertShader, nullptr);
	vkDestroyShaderModule(m_device->m_device, fragShader, nullptr);
}

void Renderer::DisposeObject(std::shared_ptr<Disposeable> object)
{
	_disposeQueue.push_back(object);
}

void Renderer::BeginFrame(uint32_t& swapchainImageIndex)
{
	m_frameNumber++;
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	_VK_CHECK(vkWaitForFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence, true, 1000000000), "--");
	_VK_CHECK(vkResetFences(m_device->m_device, 1, &m_device->GetCurrentFrame()._renderFence), "--");

	//request image from the swapchain, one second timeout
	_VK_CHECK(
		vkAcquireNextImageKHR(m_device->m_device, m_device->m_swapchain, 1000000000, m_device->GetCurrentFrame()._presentSemaphore, nullptr, &swapchainImageIndex), "XXX");

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

	_VK_CHECK(vkBeginCommandBuffer(m_device->GetCurrentFrame()._mainCommandBuffer, &cmdBeginInfo), "Cannot begin cmdBuffer");


	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	m_flash += Time::GetDelta() * (10 * m_rotationSpeed);
	float flash = abs(sin(m_flash / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;
	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = m_device->m_renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = m_device->m_vkb_swapchain.extent;
	rpInfo.framebuffer = m_device->m_frameBuffers[swapchainImageIndex];

	//connect clear values
	rpInfo.clearValueCount = 2;
	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::Draw(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, int index)
{
	if(m_material_ptr != material)
	{
		m_material_ptr = material;
		vkCmdBindPipeline(m_device->GetCurrentFrame()._mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
		uint32_t uniform_offset = m_device->PadUniformBufferSize(sizeof(GPUSceneData)) * m_device->m_imageIndex;
		vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, 
			&m_device->GetCurrentFrame()._globalDescriptor, 1, &uniform_offset);
		/*
		//object data descriptor
		vkCmdBindDescriptorSets(m_device->GetCurrentFrame()._mainCommandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 1, 1, 
			&m_device->GetCurrentFrame()._objectDescriptor, 0, nullptr);
		*/
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(m_device->GetCurrentFrame()._mainCommandBuffer, 
		0, 1, &mesh->_vertexBuffer._buffer, &offset);
	vkCmdDraw(m_device->GetCurrentFrame()._mainCommandBuffer, 
		mesh->_vertices.size(), 1, 0, 0);
}

void Renderer::EndFrame(uint32_t& swapchainImageIndex)
{
	//finalize the render pass
	vkCmdEndRenderPass(m_device->GetCurrentFrame()._mainCommandBuffer);
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

	_VK_CHECK(vkQueuePresentKHR(m_device->m_graphicsQueue, &presentInfo), "Failed to present");

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
	vkCmdPushConstants(m_device->GetCurrentFrame()._mainCommandBuffer, material->pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &push_constants);
}

void Renderer::UpdateGlobalUbo(GPUCameraData src_bufferData)
{
	void* data;
	vmaMapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._cameraBuffer._allocation, &data);
	memcpy(data, &src_bufferData, sizeof(GPUCameraData));
	vmaUnmapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._cameraBuffer._allocation);
}

void Renderer::UpdateSceneDataUbo()
{
	float framed = (m_frameNumber / 120.f);
	m_device->m_sceneParameters.ambientColor = { sin(framed),0,cos(framed),1 };
	int frameIndex = m_frameNumber % m_device->m_imageCount;
	char* sceneData;
	vmaMapMemory(m_device->m_allocator, m_device->m_sceneParameterBuffer._allocation, (void**)&sceneData);
	sceneData += m_device->PadUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;
	memcpy(sceneData, &m_device->m_sceneParameters, sizeof(GPUSceneData));
	vmaUnmapMemory(m_device->m_allocator, m_device ->m_sceneParameterBuffer._allocation);
}

void Renderer::UpdateObjectData(void* src_data, size_t size)
{
	void* objectData;
	vmaMapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._objectBuffer._allocation, &objectData);
	memcpy(objectData, src_data, size);
	vmaUnmapMemory(m_device->m_allocator, m_device->GetCurrentFrame()._objectBuffer._allocation);
}
}
