#include "PipelineHelper.h"
#include <vulkan/vulkan.h>

VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module) {

	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;

	//shader stage
	info.stage = stage;
	//module containing the code for this shader stage
	info.module = shader_module;
	//the entry point of the shader
	info.pName = "main";
	return info;
}

VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo() {
	VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;

	//no vertex bindings or attributes
	info.vertexBindingDescriptionCount = 0;
	info.vertexAttributeDescriptionCount = 0;
	return info;
}

VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology) {
	VkPipelineInputAssemblyStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.topology = topology;
	//we are not going to use primitive restart on the entire tutorial so leave it on false
	info.primitiveRestartEnable = VK_FALSE;
	return info;
}
VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygon_mode)
{
	VkPipelineRasterizationStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthClampEnable = VK_FALSE;
	//discards all primitives before the rasterization stage if enabled which we don't want
	info.rasterizerDiscardEnable = VK_FALSE;

	info.polygonMode = polygon_mode;
	info.lineWidth = 1.0f;
	//no backface cull
	info.cullMode = VK_CULL_MODE_NONE;
	info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//no depth bias
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo()
{
	VkPipelineMultisampleStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.sampleShadingEnable = VK_FALSE;
	//multisampling defaulted to no multisampling (1 sample per pixel)
	info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	info.minSampleShading = 1.0f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable = VK_FALSE;
	return info;
}

VkPipelineColorBlendAttachmentState ColorBlendAttachmentState() {
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo() {
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

	//empty defaults
	info.flags = 0;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;
	return info;
}

VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
{
	VkPipelineDepthStencilStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f; // Optional
	info.maxDepthBounds = 1.0f; // Optional
	info.stencilTestEnable = VK_FALSE;

	return info;
}
