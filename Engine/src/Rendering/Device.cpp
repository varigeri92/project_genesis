#define VK_USE_PLATFORM_WIN32_KHR
#include "Device.h"
#include <algorithm>
#include "vklog.h"
#include "../Window/Window.h"
#include "Helpers/VkInitializer.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace gns::rendering
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            LOG_VK_VERBOSE(pCallbackData->pMessage);
        }

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            LOG_VK_INFO(pCallbackData->pMessage);
        }

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            LOG_VK_WARNING(pCallbackData->pMessage);
        }

        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            LOG_VK_ERROR(pCallbackData->pMessage);
        }


        return VK_FALSE;
    }

    void UploadContext::Destroy(VkDevice device)
    {
        vkDestroyFence(device, _uploadFence, nullptr);
        vkDestroyCommandPool(device, _commandPool, nullptr);
    }

	Device::Device(Window* window): m_window(window)
	{
        LOG_INFO("Initialize Vulkan!");
        if (!InitVulkan())
        {
            LOG_ERROR("Vulkan initialization failed!");
            return;
        }
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_device;
        allocatorInfo.instance = m_instance;
        vmaCreateAllocator(&allocatorInfo, &m_allocator);

        m_gpuProperties = m_vkb_device.physical_device.properties;
        LOG_INFO("The GPU has a minimum buffer alignment of " << m_gpuProperties.limits.minUniformBufferOffsetAlignment);

        CreateSwapchain();
        m_frames.resize(m_imageCount);

        CreateCommandPool();
        InitDefaultRenderPass();
        InitGUIRenderPass();
        InitFrameBuffers();
        InitSyncStructures();
        InitDescriptors();
        
	}

	Device::~Device()
	{
        LOG_INFO("Clenup Device");
        m_uploadContext.Destroy(m_device);

        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_globalSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_objectSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_singleTextureSetLayout, nullptr);
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        vkDestroyRenderPass(m_device, m_guiPass, nullptr);
        for (int i = 0; i < m_frameBuffers.size(); i++) {
            vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
            vkDestroyImageView(m_device, m_imageViews[i], nullptr);
        }
        m_sceneParameterBuffer.Dispose(this);
        for(size_t i = 0; i<m_imageCount; i++)
        {
	        vkDestroyFence(m_device, m_frames[i]._renderFence, nullptr);
	        vkDestroySemaphore(m_device, m_frames[i]._renderSemaphore, nullptr);
	        vkDestroySemaphore(m_device, m_frames[i]._presentSemaphore, nullptr);
    		vkDestroyCommandPool(m_device, m_frames[i]._commandPool, nullptr);
            m_frames[i]._cameraBuffer.Dispose(this);
            m_frames[i]._objectBuffer.Dispose(this);
        }

    	vkDestroyImageView(m_device, _depthImageView, nullptr);
        vmaDestroyImage(m_allocator, _depthImage._image, _depthImage._allocation);
        vmaDestroyAllocator(m_allocator);
        vkb::destroy_swapchain(m_vkb_swapchain);
        vkb::destroy_device(m_vkb_device);
	}

	bool Device::InitVulkan() {
        vkb::InstanceBuilder builder;
        auto inst_ret = builder.request_validation_layers(true).set_debug_callback(debugCallback)
    		.set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            .set_app_name("Awesome Vulkan Application")
            .set_engine_name("Excellent Game Engine")
            .require_api_version(1, 2, 0)
			.set_app_version(0,0,1)
    	.build();
        if (!inst_ret) {
            LOG_ERROR("Failed to create Vulkan instance. Error: " << inst_ret.error().message());
            return false;
        }
        m_vkb_instance = inst_ret.value();
        m_instance = m_vkb_instance.instance;
        CreateSurface();
        vkb::PhysicalDeviceSelector selector{ m_vkb_instance };
        auto phys_ret = selector.set_surface(m_surface)
            .set_minimum_version(1, 1) // require a vulkan 1.1 capable device
            .require_dedicated_transfer_queue()
            .select();
        if (!phys_ret) {
            LOG_ERROR("Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message());
            return false;
        }
        m_physicalDevice = phys_ret.value();
        vkb::DeviceBuilder device_builder{ phys_ret.value() };
        VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
        shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        shader_draw_parameters_features.pNext = nullptr;
        shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;
        vkb::Device vkbDevice = device_builder.add_pNext(&shader_draw_parameters_features).build().value();
        auto dev_ret = device_builder.build();
        if (!dev_ret) {
            LOG_ERROR("Failed to create Vulkan device. Error: " << dev_ret.error().message());
            return false;
        }
        m_vkb_device = dev_ret.value();

        // Get the VkDevice handle used in the rest of a vulkan application
        m_device = m_vkb_device.device;

        // Get the graphics queue with a helper function
        auto graphics_queue_ret = m_vkb_device.get_queue(vkb::QueueType::graphics);
        if (!graphics_queue_ret) {
            LOG_ERROR("Failed to get graphics queue. Error: " << graphics_queue_ret.error().message());
            return false;
        }
        m_graphicsQueue = graphics_queue_ret.value();
        m_graphicsFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::graphics).value();

        auto transfer_queue_ret = m_vkb_device.get_queue(vkb::QueueType::transfer);
        if (!transfer_queue_ret) {
            LOG_ERROR("Failed to get graphics queue. Error: " << transfer_queue_ret.error().message());
            return false;
        }
        m_transferQueue = transfer_queue_ret.value();
        m_transferFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::transfer).value();

        auto present_queue_ret = m_vkb_device.get_queue(vkb::QueueType::present);
        if (!present_queue_ret) {
            LOG_ERROR("Failed to get graphics queue. Error: " << present_queue_ret.error().message());
            return false;
        }
        m_presentQueue = present_queue_ret.value();
        m_presentFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::present).value();


        auto compute_queue_ret = m_vkb_device.get_queue(vkb::QueueType::compute);
        if (!compute_queue_ret) {
            LOG_ERROR("Failed to get graphics queue. Error: " << compute_queue_ret.error().message());
            return false;
        }
        m_computeQueue = compute_queue_ret.value();
        m_computeFamilyIndex = m_vkb_device.get_queue_index(vkb::QueueType::compute).value();

        // Turned 400-500 lines of boilerplate into less than fifty.
        return true;
    }

    void Device::CreateSurface()
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = m_window->hwndHandle;
        createInfo.hinstance = m_window->hinstance;

        _VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface), "Failed to create Surface");
    }

    void Device::CreateSwapchain()
    {
        vkb::SwapchainBuilder swapchain_builder{ m_vkb_device };
        auto swap_ret = swapchain_builder.build();
        if (!swap_ret) {
            LOG_ERROR("Failed to build Swapchain!");
        }
        m_vkb_swapchain = swap_ret.value();
        m_imageCount = m_vkb_swapchain.image_count;
        m_swapchain = m_vkb_swapchain.swapchain;
        m_swapchainFormat = m_vkb_swapchain.image_format;
        m_imageViews = m_vkb_swapchain.get_image_views().value();

        int w= 0, h = 0;
        m_window->GetExtent(w,h);
        VkExtent3D depthImageExtent = {static_cast<uint32_t>(w),static_cast<uint32_t>(h),1};
        m_depthFormat = VK_FORMAT_D32_SFLOAT;
        VkImageCreateInfo dimg_info = ImageCreateInfo(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

        VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
        _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &_depthImageView), "Failed to create ImageView!");
    }

    void Device::RebuildSwapchain()
    {
        vkb::SwapchainBuilder swapchain_builder{ m_vkb_device };
        auto swap_ret = swapchain_builder.set_old_swapchain(m_vkb_swapchain)
            .build();
        if(!swap_ret) {
            LOG_ERROR("Failed to createSwapchain!");
            // If it failed to create a swapchain, the old swapchain handle is invalid.
            m_vkb_swapchain.swapchain = VK_NULL_HANDLE;
        }
        // Even though we recycled the previous swapchain, we need to free its resources.
        vkb::destroy_swapchain(m_vkb_swapchain);
        // Get the new swapchain and place it in our variable
        m_vkb_swapchain = swap_ret.value();
        m_swapchain = m_vkb_swapchain.swapchain;
        m_swapchainFormat = m_vkb_swapchain.image_format;
        m_imageViews = m_vkb_swapchain.get_image_views().value();

        int w = 0, h = 0;
        m_window->GetExtent(w, h);
        VkExtent3D depthImageExtent = { static_cast<uint32_t>(w),static_cast<uint32_t>(h),1 };
        VkImageCreateInfo dimg_info = ImageCreateInfo(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
        vmaDestroyImage(m_allocator, _depthImage._image, nullptr);
        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);
        VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
        vkDestroyImageView(m_device, _depthImageView, nullptr);
        _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &_depthImageView), "Failed to create ImageView!");

        for (int i = 0; i < m_frameBuffers.size(); i++) {
            vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
        }
        InitFrameBuffers();
    }

    void Device::CreateCommandPool()
    {
        VkCommandPoolCreateInfo commandPoolInfo = CommandPoolCreateInfo(m_graphicsFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        for (uint32_t i = 0; i < m_imageCount; i++)
        {
			_VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_frames[i]._commandPool), "Failed to create CommandPool");
			VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(m_frames[i]._commandPool, 1);
			_VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i]._mainCommandBuffer), "Failed to allocate Command buffer");
        }


    	//create pool for upload context
        VkCommandPoolCreateInfo uploadCommandPoolInfo = CommandPoolCreateInfo(m_graphicsFamilyIndex);
        _VK_CHECK(vkCreateCommandPool(m_device, &uploadCommandPoolInfo, nullptr, &m_uploadContext._commandPool), 
            "Failed to create upload CommandPool");
        VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(m_uploadContext._commandPool, 1);
        VkCommandBuffer cmd;
        _VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_uploadContext._commandBuffer), 
            "Failed to allocate Command buffer");

    }

    void Device::InitDefaultRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        
        colorAttachment.format = m_swapchainFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment = {};
        // Depth attachment
        depthAttachment.flags = 0;
        depthAttachment.format = m_depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    	VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = &attachments[0];
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;


        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depthDependency = {};
        depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthDependency.dstSubpass = 0;
        depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.srcAccessMask = 0;
        depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


        VkSubpassDependency dependencies[2] = { dependency, depthDependency };

        //other code...
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = &dependencies[0];

        _VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass), "Failed to create RenderPass");
    }

    void Device::InitGUIRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};

        colorAttachment.format = m_swapchainFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment = {};
        // Depth attachment
        depthAttachment.flags = 0;
        depthAttachment.format = m_depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = &attachments[0];
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;


        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depthDependency = {};
        depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depthDependency.dstSubpass = 0;
        depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.srcAccessMask = 0;
        depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


        VkSubpassDependency dependencies[2] = { dependency, depthDependency };

        //other code...
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = &dependencies[0];

        _VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_guiPass), "Failed to create RenderPass");

    }

    void Device::InitFrameBuffers()
    {
        //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
        VkFramebufferCreateInfo fb_info = {};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;

        fb_info.renderPass = m_renderPass;
        fb_info.attachmentCount = 1;
        fb_info.width = m_vkb_swapchain.extent.width;
        fb_info.height = m_vkb_swapchain.extent.height;
        fb_info.layers = 1;

        //grab how many images we have in the swapchain
        const uint32_t swapchain_imagecount = static_cast<uint32_t>(m_imageViews.size());
        m_frameBuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

        //create framebuffers for each of the swapchain image views
        for (uint32_t i = 0; i < swapchain_imagecount; i++) {

            VkImageView attachments[2];
            attachments[0] = m_imageViews[i];
            attachments[1] = _depthImageView;

            fb_info.pAttachments = attachments;
            fb_info.attachmentCount = 2;

            _VK_CHECK(vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_frameBuffers[i]), "Failed to create Frame buffers");
        }
    }

    void Device::InitSyncStructures()
    {
        //create synchronization structures

        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        //for the semaphores we don't need any flags
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        for (size_t i = 0; i<m_imageCount; i++)
        {
	        _VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frames[i]._renderFence), "Failed To create Fence");
	        _VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i]._presentSemaphore), "Failed To create presentSemaphore");
	        _VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i]._renderSemaphore), "Failed To create renderSemaphore");
        }
        VkFenceCreateInfo uploadFenceCreateInfo = {};
        uploadFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        uploadFenceCreateInfo.pNext = nullptr;
        //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        uploadFenceCreateInfo.flags = 0;

        _VK_CHECK(vkCreateFence(m_device, &uploadFenceCreateInfo, nullptr, &m_uploadContext._uploadFence), "Failed to initialize Upload Fence");
    }

    void Device::InitDescriptors()
    {

        const std::vector<VkDescriptorPoolSize> sizes =
        {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = 10;
        pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
        pool_info.pPoolSizes = sizes.data();

        _VK_CHECK(
            vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool), "Failed to create Descriptor Pool!");

        VkDescriptorSetLayoutBinding objectBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
        VkDescriptorSetLayoutCreateInfo set2info = {};
        set2info.bindingCount = 1;
        set2info.flags = 0;
        set2info.pNext = nullptr;
        set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        set2info.pBindings = &objectBind;

        vkCreateDescriptorSetLayout(m_device, &set2info, nullptr, &m_objectSetLayout);

        VkDescriptorSetLayoutBinding cameraBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
        VkDescriptorSetLayoutBinding sceneBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    	const std::vector < VkDescriptorSetLayoutBinding> bindings = { cameraBind, sceneBind };

        VkDescriptorSetLayoutCreateInfo setInfo = {};
        setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        setInfo.flags = 0;
        setInfo.pBindings = bindings.data();
        setInfo.pNext = nullptr;
        _VK_CHECK(vkCreateDescriptorSetLayout(m_device, &setInfo, nullptr, &m_globalSetLayout), "Failed To create descriptor set layout!");

        //another set, one that holds a single texture
        VkDescriptorSetLayoutBinding textureBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        VkDescriptorSetLayoutCreateInfo set3info = {};
        set3info.bindingCount = 1;
        set3info.flags = 0;
        set3info.pNext = nullptr;
        set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        set3info.pBindings = &textureBind;
        vkCreateDescriptorSetLayout(m_device, &set3info, nullptr, &m_singleTextureSetLayout);

        const size_t sceneParamBufferSize = m_imageCount * PadUniformBufferSize(sizeof(GPUSceneData));
        m_sceneParameterBuffer = CreateBuffer(m_allocator, sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        constexpr uint32_t structSize = sizeof(GPUCameraData);
        LOG_INFO("Structure Size: " << structSize);
        for (uint32_t i = 0; i < m_imageCount; i++)
        {
            m_frames[i]._cameraBuffer = CreateBuffer(m_allocator, structSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

            m_frames[i]._objectBuffer = CreateBuffer(m_allocator, sizeof(GPUObjectData) * MAX_OBJECTS,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

            VkDescriptorSetAllocateInfo allocInfo = {};
            allocInfo.pNext = nullptr;
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_globalSetLayout;
            vkAllocateDescriptorSets(m_device, &allocInfo, &m_frames[i]._globalDescriptor);

            //allocate the descriptor set that will point to object buffer
            VkDescriptorSetAllocateInfo objectSetAlloc = {};
            objectSetAlloc.pNext = nullptr;
            objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            objectSetAlloc.descriptorPool = m_descriptorPool;
            objectSetAlloc.descriptorSetCount = 1;
            objectSetAlloc.pSetLayouts = &m_objectSetLayout;
            vkAllocateDescriptorSets(m_device, &objectSetAlloc, &m_frames[i]._objectDescriptor);

            VkDescriptorBufferInfo cameraInfo = {};
            cameraInfo.buffer = m_frames[i]._cameraBuffer._buffer;
            cameraInfo.offset = 0;
            cameraInfo.range = structSize;

            VkDescriptorBufferInfo sceneInfo;
            sceneInfo.buffer = m_sceneParameterBuffer._buffer;
            sceneInfo.offset = 0;
            sceneInfo.range = sizeof(GPUSceneData);

            VkDescriptorBufferInfo objectBufferInfo;
            objectBufferInfo.buffer = m_frames[i]._objectBuffer._buffer;
            objectBufferInfo.offset = 0;
            objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

            VkWriteDescriptorSet cameraWrite = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                m_frames[i]._globalDescriptor, &cameraInfo, 0);

            VkWriteDescriptorSet sceneWrite = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                m_frames[i]._globalDescriptor, &sceneInfo, 1);

            VkWriteDescriptorSet objectWrite = WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, m_frames[i]._objectDescriptor,
                &objectBufferInfo, 0);

        	std::vector<VkWriteDescriptorSet> setWrites = { cameraWrite, sceneWrite, objectWrite };
            const size_t writesCount = setWrites.size();
            vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writesCount), setWrites.data(), 0, nullptr);
        }
    }

    size_t Device::PadUniformBufferSize(size_t originalSize)
    {
        // Calculate required alignment based on minimum device offset alignment
        size_t minUboAlignment = m_gpuProperties.limits.minUniformBufferOffsetAlignment;
        size_t alignedSize = originalSize;
        if (minUboAlignment > 0) {
            alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
        }
        return alignedSize;
    }

    void Device::EndFrame()
    {
        m_imageIndex++;
        m_imageIndex = m_imageIndex % m_imageCount;
    }

    FrameData& Device::GetCurrentFrame()
    {
        return m_frames[m_imageIndex];
    }

    void Device::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, bool transfer/* = true*/)
    {
        VkCommandBuffer cmd = m_uploadContext._commandBuffer;

        //begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
        VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        _VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo),"FailedTo Begin CommandBuffer");
        function(cmd);
        _VK_CHECK(vkEndCommandBuffer(cmd), "Failed To end CommandBuffer");
        VkSubmitInfo submit = SubmitInfo(&cmd);

        _VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext._uploadFence), "Failed To bubmit Command Buffer");

        vkWaitForFences(m_device, 1, &m_uploadContext._uploadFence, VK_TRUE, static_cast<uint64_t>(-1));
        vkResetFences(m_device, 1, &m_uploadContext._uploadFence);
        vkResetCommandPool(m_device, m_uploadContext._commandPool, 0);
    }
}
