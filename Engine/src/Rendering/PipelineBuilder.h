#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace gns::rendering
{
	class Device;
	class Renderer;

	class PipelineBuilder
	{
		friend class Renderer;
	public:
		PipelineBuilder()=delete;
		PipelineBuilder(Device* device);
		~PipelineBuilder();
		PipelineBuilder operator=(PipelineBuilder& other) = delete;

	private:
		Device* m_device;
		VkPipelineCache m_pipelineCache;

		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkViewport _viewport;
		VkRect2D _scissor;
		VkPipelineRasterizationStateCreateInfo _rasterizer;
		VkPipelineColorBlendAttachmentState _colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo _multisampling;
		VkPipelineLayout _pipelineLayout;
		VkPipelineDepthStencilStateCreateInfo _depthStencil;

		VkPipeline CreateDefaultPipeline(VkDevice device, VkRenderPass pass);
		VkPipeline Build(VkDevice device, VkRenderPass pass);
};
}