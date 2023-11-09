#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "Device.h"
#include "../AssetDatabase/Guid.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace gns::rendering
{
	struct Material;

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class PipelineBuilder
	{
		friend class Renderer;
		friend class Texture;
		struct PipelineData
		{
			VkPipeline pipeline;
			VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

			VkShaderModule vertexModule = VK_NULL_HANDLE;
			VkShaderModule fragmentModule = VK_NULL_HANDLE;
			VkRenderPass renderPass = VK_NULL_HANDLE;
			VkPipelineShaderStageCreateInfo vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo;
			VkDescriptorSetLayout descriptorSetLayout;
		};

	public:
		PipelineBuilder() = delete;
		PipelineBuilder(Device* device);
		~PipelineBuilder();

	private:
		Device* m_device;
		std::unordered_map<core::guid, PipelineData> m_pipelineCache;
		core::guid currentShaderGuid;

		//VkPipeline m_pipeline;
		//VkPipelineLayout m_pipelineLayout;
		//VkShaderModule m_vertexModule = VK_NULL_HANDLE;
		//VkShaderModule m_fragmentModule = VK_NULL_HANDLE;
		//VkRenderPass m_renderPass;
		//VkDescriptorSetLayout m_descriptorSetLayout;

		VkRenderPass GetRenderPass()
		{
			//return m_renderPass;
			return m_pipelineCache[currentShaderGuid].renderPass;
		}
		VkPipeline GetPipeline()
		{
			//return m_pipeline;
			return m_pipelineCache[currentShaderGuid].pipeline;
		}
		VkPipelineLayout GetPipelineLayout()
		{
			//return m_pipelineLayout;
			return m_pipelineCache[currentShaderGuid].pipelineLayout;
		}
		

		VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;

		
		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;
		std::vector<void*> m_uniformBuffersMapped;
		VkDescriptorPool m_descriptorPool;
		



		std::vector<char> ReadFile(const std::string& filename);
		void CreateShaderModule_Internal(const std::vector<char>& code, VkShaderModule& out_module);
		void BuildPipeline(Material& material);
		void CreateRenderPass();
		void CreateShaderModules(std::string vertexShader, std::string fragmentShader);
		core::guid CreateShaderModules(Material& material);
		void CreateShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits flags, 
			VkPipelineShaderStageCreateInfo& out_createInfoStruct);
		void CreateVertexInputInfo(uint32_t bindingDescriptionCount, VkVertexInputBindingDescription* bindingDescription,
			uint32_t attributeDescriptionCount, VkVertexInputAttributeDescription* attributeDescription);

		VkPipelineDynamicStateCreateInfo CreateDynamicStates(uint32_t size, const VkDynamicState* states);
		VkPipelineInputAssemblyStateCreateInfo CrateInputAssemblyInfo();

		VkPipelineViewportStateCreateInfo CreateViewportState();
		VkPipelineRasterizationStateCreateInfo CreateRasterizationState();
		VkPipelineMultisampleStateCreateInfo CreateMultisampleState();
		VkPipelineColorBlendAttachmentState CreateColorBlendAttachment();
		VkPipelineColorBlendStateCreateInfo CreateBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment);
		VkPipelineDepthStencilStateCreateInfo CreateDepthStencilState();

		void CreateDescriptorSetLayout();
		void CreatePipelineLayout();

		void BildPipeline_Internal();


		void CreateDescriptorPool();
		void CreateDescriptorSets(VkImageView textureImageView, VkSampler textureSampler, Texture& texture);
		void CreateUniformBuffers();

		void CleanupPipeline();

		void CreateDepthResources();
		void CleanupDepthResources();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, 
			VkFormatFeatureFlags features);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);
	};

}
