﻿#include "gnspch.h"
#include "PipelineBuilder.h"


namespace gns::rendering
{

	PipelineBuilder::PipelineBuilder(Device* device) : m_device(device) {}
	PipelineBuilder::~PipelineBuilder(){}

	VkPipeline PipelineBuilder::Build(VkDevice device, VkRenderPass pass)
	{
		//make viewport state from our stored viewport and scissor.
			//at the moment we won't support multiple viewports or scissors
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &_viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &_scissor;
		//viewportState.flags = VK_DYNAMIC_STATE_VIEWPORT;
		//setup dummy color blending. We aren't using transparent objects yet
		//the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &_colorBlendAttachment;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynStateInfo = {};
		dynStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynStateInfo.flags = 0;
		dynStateInfo.dynamicStateCount = dynamicStates.size();
		dynStateInfo.pDynamicStates = dynamicStates.data();
		dynStateInfo.pNext = nullptr;


		//build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stageCount = _shaderStages.size();
		pipelineInfo.pStages = _shaderStages.data();
		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &_depthStencil;
		pipelineInfo.pDynamicState = &dynStateInfo;



		//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
			std::cout << "failed to create pipeline\n";
			return VK_NULL_HANDLE; // failed to create graphics pipeline
		}
		else
		{
			return newPipeline;
		}
	}

}