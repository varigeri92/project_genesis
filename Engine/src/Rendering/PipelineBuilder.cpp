#include "PipelineBuilder.h"
#include <fstream>

#include "Log.h"
#include "vklog.h"
#include "BufferHelper.hpp"
#include <array>
#include "VulkanImage.h"
#include "Material.h"
#include "Texture.h"
#include "Renderer.h"
namespace gns::rendering
{
    PipelineBuilder::PipelineBuilder(Device* device) :
        m_device{ device },
		m_vertexInputInfo {}
    {};

    PipelineBuilder::~PipelineBuilder()
    {
        CleanupPipeline();
    }
    

    std::vector<char> PipelineBuilder::ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            LOG_ERROR("FAILED TO OPEN FILE: " << filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        LOG_INFO("SIZE: "<< fileSize<< " NAME: " << filename);
        return buffer;
    }

    void PipelineBuilder::CreateShaderModule_Internal(const std::vector<char>& code, VkShaderModule& out_module)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        _VK_CHECK(vkCreateShaderModule(m_device->GetDevice(), &createInfo, nullptr, &out_module), "failed to create shader module!");
    }

    void PipelineBuilder::BuildPipeline(Material& material)
    {
        if (m_pipelineCache.find(material.shader->guid) != m_pipelineCache.end()) {
            currentShaderGuid = material.shader->guid;
            return;
        }

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        CreateVertexInputInfo(1, &bindingDescription,
            static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data());

        currentShaderGuid = CreateShaderModules(material);
        CreateShaderStage(m_pipelineCache[currentShaderGuid].vertexModule, VK_SHADER_STAGE_VERTEX_BIT, m_pipelineCache[currentShaderGuid].vertShaderStageInfo);
        CreateShaderStage(m_pipelineCache[currentShaderGuid].fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT, m_pipelineCache[currentShaderGuid].fragShaderStageInfo);
        VkPipelineShaderStageCreateInfo shaderStages[] = {
        	m_pipelineCache[currentShaderGuid].vertShaderStageInfo,
        	m_pipelineCache[currentShaderGuid].fragShaderStageInfo
        };

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = CreateDynamicStates(static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = CrateInputAssemblyInfo();
        VkPipelineViewportStateCreateInfo viewportState = CreateViewportState();
        VkPipelineRasterizationStateCreateInfo rasterizer = CreateRasterizationState();
        VkPipelineMultisampleStateCreateInfo multisampling = CreateMultisampleState();
        VkPipelineColorBlendAttachmentState colorBlendAttachment = CreateColorBlendAttachment();
        VkPipelineColorBlendStateCreateInfo colorBlending = CreateBlendState(colorBlendAttachment);
        VkPipelineDepthStencilStateCreateInfo depthStencil = CreateDepthStencilState();


        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDepthResources();
        CreateDescriptorSetLayout();
        CreatePipelineLayout();
        CreateRenderPass();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &m_vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineCache[currentShaderGuid].pipelineLayout;
        pipelineInfo.renderPass = m_pipelineCache[currentShaderGuid].renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional

        _VK_CHECK(vkCreateGraphicsPipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipelineCache[currentShaderGuid].pipeline),
            "failed to create graphics pipeline!");
    }

    void PipelineBuilder::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_device->GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        const std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        _VK_CHECK(vkCreateRenderPass(m_device->GetDevice(), &renderPassInfo, nullptr, &m_pipelineCache[currentShaderGuid].renderPass),
            "failed to create render pass!")
    }

    void PipelineBuilder::CreateShaderModules(std::string vertexShader, std::string fragmentShader)
    {
        auto vertShaderCode = ReadFile(vertexShader);
        CreateShaderModule_Internal(vertShaderCode, m_pipelineCache[currentShaderGuid].vertexModule);
        auto fragShaderCode = ReadFile(fragmentShader);
        CreateShaderModule_Internal(fragShaderCode, m_pipelineCache[currentShaderGuid].fragmentModule);
        
    }

    core::guid PipelineBuilder::CreateShaderModules(Material& material)
    {
        core::guid shaderGuid = material.shader->guid;
        m_pipelineCache[shaderGuid] = {};

        auto vertShaderCode = ReadFile(material.shader->vertex_path);
        CreateShaderModule_Internal(vertShaderCode, m_pipelineCache[shaderGuid].vertexModule);
        auto fragShaderCode = ReadFile(material.shader->fragment_path);
        CreateShaderModule_Internal(fragShaderCode, m_pipelineCache[shaderGuid].fragmentModule);

        return shaderGuid;
    }

    void PipelineBuilder::CreateShaderStage(VkShaderModule shaderModule, VkShaderStageFlagBits flags,
                                            VkPipelineShaderStageCreateInfo& out_createInfoStruct)
    {
        out_createInfoStruct.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        out_createInfoStruct.stage = flags;
        out_createInfoStruct.module = shaderModule;
        out_createInfoStruct.pName = "main";
    }

    void PipelineBuilder::CreateVertexInputInfo(uint32_t bindingDescriptionCount,
	    VkVertexInputBindingDescription* bindingDescription, uint32_t attributeDescriptionCount,
	    VkVertexInputAttributeDescription* attributeDescription)
    {
        m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        m_vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptionCount;
        m_vertexInputInfo.pVertexBindingDescriptions = bindingDescription; // Optional
        m_vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionCount;
        m_vertexInputInfo.pVertexAttributeDescriptions = attributeDescription; // Optional
    }

    VkPipelineDynamicStateCreateInfo PipelineBuilder::CreateDynamicStates(uint32_t size, const VkDynamicState* states)
    {
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = size;
        dynamicState.pDynamicStates = states;
        return dynamicState;
    }

    VkPipelineInputAssemblyStateCreateInfo PipelineBuilder::CrateInputAssemblyInfo()
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        return inputAssembly;
    }

    VkPipelineViewportStateCreateInfo PipelineBuilder::CreateViewportState()
    {

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_device->GetSwapchainExtent().width;
        viewport.height = (float)m_device->GetSwapchainExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_device->GetSwapchainExtent();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        return viewportState;
    }

    VkPipelineRasterizationStateCreateInfo PipelineBuilder::CreateRasterizationState()
    {
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
        return rasterizer;
    }

    VkPipelineMultisampleStateCreateInfo PipelineBuilder::CreateMultisampleState()
    {
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        return multisampling;
    }

    VkPipelineColorBlendAttachmentState PipelineBuilder::CreateColorBlendAttachment()
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
        return colorBlendAttachment;
    }

    VkPipelineColorBlendStateCreateInfo PipelineBuilder::CreateBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment)
    {
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        return colorBlending;
    }

    VkPipelineDepthStencilStateCreateInfo PipelineBuilder::CreateDepthStencilState()
    {
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional
        return depthStencil;
    }

    void PipelineBuilder::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

    	_VK_CHECK(vkCreateDescriptorSetLayout(m_device->GetDevice(), &layoutInfo, nullptr, &m_pipelineCache[currentShaderGuid].descriptorSetLayout), 
            "failed to create descriptor set layout!")
    }

    void PipelineBuilder::CreatePipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_pipelineCache[currentShaderGuid].descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        _VK_CHECK(vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineCache[currentShaderGuid].pipelineLayout),
            "failed to create pipeline layout!");
    }


    void PipelineBuilder::BildPipeline_Internal()
    {
    }

    void PipelineBuilder::CreateDescriptorPool()
    {
        uint32_t count = 2;

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)* count;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)* count;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)* count;


        _VK_CHECK(vkCreateDescriptorPool(m_device->GetDevice(), &poolInfo, nullptr, &m_descriptorPool),
            "failed to create descriptor pool!")
    }

    void PipelineBuilder::CreateDescriptorSets(VkImageView textureImageView, VkSampler textureSampler, Texture& texture)
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_pipelineCache[currentShaderGuid].descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        texture.m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, texture.m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);


            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = texture.m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = texture.m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void PipelineBuilder::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            BufferHelper::CreateBuffer(m_device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_uniformBuffers[i], m_uniformBuffersMemory[i]);

            vkMapMemory(m_device->GetDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
        }
    }

    void PipelineBuilder::CleanupPipeline()
    {

        vkDestroyDescriptorPool(m_device->GetDevice(), m_descriptorPool, nullptr);

        CleanupDepthResources();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(m_device->GetDevice(), m_uniformBuffers[i], nullptr);
            vkFreeMemory(m_device->GetDevice(), m_uniformBuffersMemory[i], nullptr);
        }
        /*
        vkDestroyShaderModule(m_device->GetDevice(), m_vertexModule, nullptr);
        vkDestroyShaderModule(m_device->GetDevice(), m_fragmentModule, nullptr);
        vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
        vkDestroyPipeline(m_device->GetDevice(), m_pipeline, nullptr);
        */
         
        for (const auto& pair : m_pipelineCache) {
            const PipelineData& pipeline = pair.second;

            vkDestroyShaderModule(m_device->GetDevice(), pipeline.vertexModule, nullptr);
            vkDestroyShaderModule(m_device->GetDevice(), pipeline.fragmentModule, nullptr);
            vkDestroyPipelineLayout(m_device->GetDevice(), pipeline.pipelineLayout, nullptr);
            vkDestroyRenderPass(m_device->GetDevice(), pipeline.renderPass, nullptr);
            vkDestroyPipeline(m_device->GetDevice(), pipeline.pipeline, nullptr);
            vkDestroyDescriptorSetLayout(m_device->GetDevice(), pipeline.descriptorSetLayout, nullptr);
        }
    }

    void PipelineBuilder::CreateDepthResources()
    {
        VkFormat depthFormat = FindDepthFormat();

    	VulkanImage::CreateImage(m_device, m_device->m_swapChainExtent.width, m_device->m_swapChainExtent.height, 
            depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_depthImage, m_depthImageMemory);

    	m_depthImageView = VulkanImage::CreateImageView(m_device, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void PipelineBuilder::CleanupDepthResources()
    {
        vkDestroyImageView(m_device->GetDevice(), m_depthImageView, nullptr);
        vkDestroyImage(m_device->GetDevice(), m_depthImage, nullptr);
        vkFreeMemory(m_device->GetDevice(), m_depthImageMemory, nullptr);
    }

    VkFormat PipelineBuilder::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                                  VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_device->m_physDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        LOG_ERROR("Failed ToFind Supported ImageFormat!");
        return candidates[0];
    }

    VkFormat PipelineBuilder::FindDepthFormat()
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool PipelineBuilder::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
}
