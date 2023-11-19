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

	Device::Device(Window* window)
	{
        LOG_INFO("Initialize Vulkan!");
        if (!InitVulkan(window))
        {
            LOG_ERROR("Vulkan initialization failed!");
            return;
        }
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_device;
        allocatorInfo.instance = m_instance;
        vmaCreateAllocator(&allocatorInfo, &m_allocator);

        CreateSwapchain(window);
        m_frames.resize(m_imageCount);

        CreateCommandPool();
        InitDefaultRenderPass();
        InitFrameBuffers();
        InitSyncStructures();
        
	}

	Device::~Device()
	{
        LOG_INFO("Clenup Device");
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        for (int i = 0; i < m_frameBuffers.size(); i++) {
            vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
            vkDestroyImageView(m_device, m_imageViews[i], nullptr);
        }
        for(size_t i = 0; i<m_imageCount; i++)
        {
	        
	        vkDestroyFence(m_device, m_frames[i]._renderFence, nullptr);
	        vkDestroySemaphore(m_device, m_frames[i]._renderSemaphore, nullptr);
	        vkDestroySemaphore(m_device, m_frames[i]._presentSemaphore, nullptr);
    		vkDestroyCommandPool(m_device, m_frames[i]._commandPool, nullptr);
        }

    	vkDestroyImageView(m_device, _depthImageView, nullptr);
        vmaDestroyImage(m_allocator, _depthImage._image, _depthImage._allocation);
        vmaDestroyAllocator(m_allocator);
        vkb::destroy_swapchain(m_vkb_swapchain);
        vkb::destroy_device(m_vkb_device);
	}

	bool Device::InitVulkan(Window* window) {
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
        CreateSurface(window);
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
        // automatically propagate needed data from instance & physical device
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

    void Device::CreateSurface(Window* window)
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = window->hwndHandle;
        createInfo.hinstance = window->hinstance;

        _VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface), "Failed to create Surface");
    }

    void Device::CreateSwapchain(Window* window)
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
        window->GetExtent(w,h);
        //depth image size will match the window
        VkExtent3D depthImageExtent = {static_cast<uint32_t>(w),static_cast<uint32_t>(h),1};

        //hardcoding the depth format to 32 bit float
        m_depthFormat = VK_FORMAT_D32_SFLOAT;

        //the depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo dimg_info = image_create_info(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

        //for the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //allocate and create the image
        vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

        //build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo dview_info = imageview_create_info(m_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);

        _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &_depthImageView), "Failed to create ImageView!");
    }

    void Device::RebuildSwapchain()
    {
        vkb::SwapchainBuilder swapchain_builder{ m_vkb_device };
        auto swap_ret = swapchain_builder.set_old_swapchain(m_vkb_swapchain)
            .build();
        if(!swap_ret) {
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
    }

    void Device::CreateCommandPool()
    {
        VkCommandPoolCreateInfo commandPoolInfo = CommandPoolCreateInfo(m_graphicsFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        for (int i = 0; i < m_imageCount; i++)
        {
			_VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_frames[i]._commandPool), "Failed to create CommandPool");
			VkCommandBufferAllocateInfo cmdAllocInfo = CommandBufferAllocateInfo(m_frames[i]._commandPool, 1);
			_VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i]._mainCommandBuffer), "Failed to allocate Command buffer");
	        
        }

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
        for (int i = 0; i < swapchain_imagecount; i++) {

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
}
