#include "gnspch.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "Device.h"
#include "Log.h"
#include "Utils/VkDebug.h"
#include "../Window/Window.h"
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include "Utils/VkHelpers.h"


void gns::rendering::UploadContext::Destroy(VkDevice device)
{
    PROFILE_FUNC
    vkDestroyFence(device, _uploadFence, nullptr);
    vkDestroyCommandPool(device, _commandPool, nullptr);
}

gns::rendering::Device::Device(Window* window) : m_window(window)
{
    PROFILE_FUNC
	m_imageIndex = 0;
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
    LOG_INFO(m_imageCount);

    CreateCommandPool();
    InitSyncStructures();

	CreateRenderTarget(1920, 1080);
	InitTextureRenderPass();
    InitOffscreenFrameBuffers();

    InitGUIRenderPass();
	InitFrameBuffers();

    CreateDescriptorPool();
    //InitRenderTargetDescriptor();
}

gns::rendering::Device::~Device()
{
    PROFILE_FUNC
    LOG_INFO("Clenup Device");
    m_uploadContext.Destroy(m_device);

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_globalSetLayout, nullptr);
    //vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    vkDestroyRenderPass(m_device, m_screenPass.renderPass, nullptr);
	vkDestroyRenderPass(m_device, m_offscreenPass.renderPass, nullptr);
	
    for (int i = 0; i < m_offscreenPass.frameBuffers.size(); i++) {
        vkDestroyFramebuffer(m_device, m_screenPass.frameBuffers[i], nullptr);
        vkDestroyImageView(m_device, m_imageViews[i], nullptr);
		vkDestroyFramebuffer(m_device, m_offscreenPass.frameBuffers[i], nullptr);
    }


	vkDestroySampler(m_device, m_offscreenPass.sampler, nullptr);
	
    DisposeBuffer(m_sceneParameterBuffer);
    for (size_t i = 0; i < m_imageCount; i++)
    {
        vkDestroyFence(m_device, m_frames[i]._renderFence, nullptr);
        vkDestroySemaphore(m_device, m_frames[i]._renderSemaphore, nullptr);
        vkDestroySemaphore(m_device, m_frames[i]._presentSemaphore, nullptr);
        vkDestroyCommandPool(m_device, m_frames[i]._commandPool, nullptr);
        DisposeBuffer(m_frames[i]._cameraBuffer);
        DisposeBuffer(m_frames[i]._objectBuffer);
    }
	
	vkDestroyImageView(m_device, _depthImageView, nullptr);
	vmaDestroyImage(m_allocator, _depthImage._image, _depthImage._allocation);
	
	vkDestroyImageView(m_device, m_offscreenPass.color.view, nullptr);
	vkDestroyImageView(m_device, m_offscreenPass.depth.view, nullptr);

    vmaDestroyImage(m_allocator, m_offscreenPass.color.image._image, m_offscreenPass.color.image._allocation);
	vmaDestroyImage(m_allocator, m_offscreenPass.depth.image._image, m_offscreenPass.depth.image._allocation);
	
	vmaDestroyAllocator(m_allocator);
    vkb::destroy_swapchain(m_vkb_swapchain);
    vkb::destroy_device(m_vkb_device);
}

bool gns::rendering::Device::InitVulkan()
{
    PROFILE_FUNC
    vkb::InstanceBuilder builder;

#ifdef VK_LOG
    auto inst_ret = builder.request_validation_layers(true).set_debug_callback(VulkanDebugCallback)
        .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        .set_app_name("Awesome Vulkan Application")
        .set_engine_name("Excellent Game Engine")
        .require_api_version(1, 2, 0)
        .set_app_version(0, 0, 1)
        .build();
#else
    auto inst_ret = builder.request_validation_layers(false).set_app_name("Awesome Vulkan Application")
        .set_engine_name("Excellent Game Engine")
        .require_api_version(1, 2, 0)
        .set_app_version(0, 0, 1)
        .build();
#endif

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
    LOG_INFO("PhysDevice Selected!");
    vkb::DeviceBuilder device_builder{ phys_ret.value() };
    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
    shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw_parameters_features.pNext = nullptr;
    shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;
    //vkb::Device vkbDevice = device_builder.add_pNext(&shader_draw_parameters_features).build().value();
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        LOG_ERROR("Failed to create Vulkan device. Error: " << dev_ret.error().message());
        return false;
    }
    m_vkb_device = dev_ret.value();

    // Get the VkDevice handle used in the rest of a vulkan application
    m_device = dev_ret.value().device;
    LOG_INFO("LogicalDevice Created!");
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

    return true;
}

void gns::rendering::Device::CreateSurface()
{
    PROFILE_FUNC
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = m_window->hwndHandle;
    createInfo.hinstance = m_window->hinstance;
    _VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface), "Failed to create Surface");
}

void gns::rendering::Device::CreateSwapchain()
{
    PROFILE_FUNC
    vkb::SwapchainBuilder swapchain_builder{ m_vkb_device };
    vkb::Result<vkb::Swapchain> swap_ret = swapchain_builder.build();
    if (!swap_ret) {
        LOG_ERROR("Failed to build Swapchain!");
    }
    m_vkb_swapchain = swap_ret.value();
    m_imageCount = m_vkb_swapchain.image_count;
    m_swapchain = m_vkb_swapchain.swapchain;
    m_swapchainFormat = m_vkb_swapchain.image_format;
    m_imageViews = m_vkb_swapchain.get_image_views().value();

    int w = 0, h = 0;
    m_window->GetExtent(w, h);
    VkExtent3D depthImageExtent = { static_cast<uint32_t>(w),static_cast<uint32_t>(h),1 };
    m_depthFormat = VK_FORMAT_D32_SFLOAT;
    VkImageCreateInfo dimg_info = ImageCreateInfo(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

    VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &_depthImageView), "Failed to create ImageView!");
}

void gns::rendering::Device::RebuildSwapchain(int width, int height)
{
    LOG_INFO("Swapchain rebuilt!");
    PROFILE_FUNC
    vkb::SwapchainBuilder swapchain_builder{ m_vkb_device };
    auto swap_ret = swapchain_builder.set_old_swapchain(m_vkb_swapchain).build();
    if (!swap_ret) {
        LOG_ERROR("Failed to createSwapchain!");
        m_vkb_swapchain.swapchain = VK_NULL_HANDLE;
    }
    // Even though we recycled the previous swapchain, we need to free its resources.
    vkb::destroy_swapchain(m_vkb_swapchain);
    // Get the new swapchain and place it in our variable
    m_vkb_swapchain = swap_ret.value();
    m_swapchain = m_vkb_swapchain.swapchain;
    m_swapchainFormat = m_vkb_swapchain.image_format;

    for (int i = 0; i < m_imageViews.size(); i++) {
        vkDestroyImageView(m_device, m_imageViews[i], nullptr);
    }
    m_imageViews = m_vkb_swapchain.get_image_views().value();

    VkExtent3D depthImageExtent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height),1 };
    VkImageCreateInfo dimg_info = ImageCreateInfo(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
    vmaDestroyImage(m_allocator, _depthImage._image, nullptr);
    vmaFreeMemory(m_allocator, _depthImage._allocation);
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);
	VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    vkDestroyImageView(m_device, _depthImageView, nullptr);
    _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &_depthImageView), "Failed to create ImageView!");

    for (int i = 0; i < m_screenPass.frameBuffers.size(); i++) {
        vkDestroyFramebuffer(m_device, m_screenPass.frameBuffers[i], nullptr);
    }
    InitFrameBuffers();
}

void gns::rendering::Device::CreateCommandPool()
{
    PROFILE_FUNC
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

void gns::rendering::Device::CreateRenderTarget(uint32_t width, uint32_t height)
{
	m_offscreenPass.frameBuffers.resize(m_imageCount);
	m_offscreenPass.width = width; 
	m_offscreenPass.height = height;

	// Color attachment
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = m_swapchainFormat;
	image.extent.width = m_offscreenPass.width;
	image.extent.height = m_offscreenPass.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    
	_VK_CHECK(vmaCreateImage(m_allocator, &image, &dimg_allocinfo, &m_offscreenPass.color.image._image, &m_offscreenPass.color.image._allocation, nullptr),
        "Failed to Create Render target Image");

	VkImageViewCreateInfo colorImageView = {};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = m_swapchainFormat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = m_offscreenPass.color.image._image;
	_VK_CHECK(vkCreateImageView(m_device, &colorImageView, nullptr, &m_offscreenPass.color.view),
		"Failed to create image View");

	// Create sampler to sample from the attachment in the fragment shader
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = samplerInfo.addressModeU;
	samplerInfo.addressModeW = samplerInfo.addressModeU;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	_VK_CHECK(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_offscreenPass.sampler),
		"Failed to create render pass sampler");

	// Depth stencil attachment
	image.format = m_depthFormat;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    _VK_CHECK(vmaCreateImage(m_allocator, &image, &dimg_allocinfo, &m_offscreenPass.depth.image._image, &m_offscreenPass.depth.image._allocation, nullptr),
        "Failed to Create Render target Image");

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = m_depthFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = m_offscreenPass.depth.image._image;
	_VK_CHECK(vkCreateImageView(m_device, &depthStencilView, nullptr, &m_offscreenPass.depth.view), "Failed to create Image!");
}

void gns::rendering::Device::InitTextureRenderPass()
{
    // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

	std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
	// Color attachment
	attchmentDescriptions[0].format = m_swapchainFormat;
	attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// Depth attachment
	attchmentDescriptions[1].format = m_depthFormat;
	attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create the actual renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
	renderPassInfo.pAttachments = attchmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	_VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_offscreenPass.renderPass),
		"Failed to create Offscreen pass");
	
}

void gns::rendering::Device::InitDefaultRenderPass()
{
    PROFILE_FUNC
    
    //**___________________----------------------____________________**//
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

    _VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_screenPass.renderPass), "Failed to create RenderPass");
}

void gns::rendering::Device::InitGUIRenderPass()
{
    PROFILE_FUNC
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

    _VK_CHECK(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_screenPass.renderPass), "Failed to create RenderPass");
}

void gns::rendering::Device::InitFrameBuffers()
{
    PROFILE_FUNC
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = nullptr;

    fb_info.renderPass = m_screenPass.renderPass;
    fb_info.attachmentCount = 1;
    fb_info.width = m_vkb_swapchain.extent.width;
    fb_info.height = m_vkb_swapchain.extent.height;
    fb_info.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchain_imagecount = static_cast<uint32_t>(m_imageViews.size());
    m_screenPass.frameBuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    //create framebuffers for each of the swapchain image views
    for (uint32_t i = 0; i < swapchain_imagecount; i++) {

        VkImageView attachments[2];
        attachments[0] = m_imageViews[i];
        attachments[1] = _depthImageView;

        fb_info.pAttachments = attachments;
        fb_info.attachmentCount = 2;

        _VK_CHECK(vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_screenPass.frameBuffers[i]), "Failed to create Frame buffers");
    }
}

void gns::rendering::Device::InitOffscreenFrameBuffers()
{
    /*/---------Create Framebuffer for Texture render-----------/*/
    //Create frame buffers for the render target images!
    VkFramebufferCreateInfo fbufCreateInfo = {};
    fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbufCreateInfo.renderPass = m_offscreenPass.renderPass;
    fbufCreateInfo.width = m_offscreenPass.width;
    fbufCreateInfo.height = m_offscreenPass.height;
    fbufCreateInfo.layers = 1;

    const uint32_t swapchain_imagecount = static_cast<uint32_t>(m_imageViews.size());
    m_offscreenPass.frameBuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    for (uint32_t i = 0; i < swapchain_imagecount; i++) {

        VkImageView attachments[2];
        //Maybe we need an image view / frame
        attachments[0] = m_offscreenPass.color.view;
        attachments[1] = m_offscreenPass.depth.view;

        fbufCreateInfo.attachmentCount = 2;
        fbufCreateInfo.pAttachments = attachments;

        _VK_CHECK(vkCreateFramebuffer(m_device, &fbufCreateInfo, nullptr, &m_offscreenPass.frameBuffers[i]),
            "Failed to create Frame buffers");
    }

    // Fill a descriptor for later use in a descriptor set
    m_offscreenPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_offscreenPass.descriptor.imageView = m_offscreenPass.color.view;
    m_offscreenPass.descriptor.sampler = m_offscreenPass.sampler;
}

void gns::rendering::Device::DestroyOffscreenFrameBuffer(uint32_t width, uint32_t height)
{
    m_offscreenPass.width = width;
    m_offscreenPass.height = height;
    vkDeviceWaitIdle(m_device);
    // depth image:
	VkExtent3D depthImageExtent = { m_offscreenPass.width ,m_offscreenPass.height,1 };
    VkImageCreateInfo dimg_info = ImageCreateInfo(m_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
    vmaDestroyImage(m_allocator, m_offscreenPass.depth.image._image, m_offscreenPass.depth.image._allocation);

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo,
        &m_offscreenPass.depth.image._image, &m_offscreenPass.depth.image._allocation, nullptr);

    VkImageViewCreateInfo dview_info = ImageViewCreateInfo(m_depthFormat, m_offscreenPass.depth.image._image, VK_IMAGE_ASPECT_DEPTH_BIT);
    vkDestroyImageView(m_device, m_offscreenPass.depth.view, nullptr);
    _VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &m_offscreenPass.depth.view), "Failed to create ImageView!");

    //Color Image
    vmaDestroyImage(m_allocator, m_offscreenPass.color.image._image, m_offscreenPass.color.image._allocation);

    VkImageCreateInfo cimg_info = {};
    cimg_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    cimg_info.imageType = VK_IMAGE_TYPE_2D;
    cimg_info.format = m_swapchainFormat;
    cimg_info.extent.width = m_offscreenPass.width;
    cimg_info.extent.height = m_offscreenPass.height;
    cimg_info.extent.depth = 1;
    cimg_info.mipLevels = 1;
    cimg_info.arrayLayers = 1;
    cimg_info.samples = VK_SAMPLE_COUNT_1_BIT;
    cimg_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    cimg_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VmaAllocationCreateInfo cimg_allocinfo = {};
    cimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    cimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vmaCreateImage(m_allocator, &cimg_info, &cimg_allocinfo,
        &m_offscreenPass.color.image._image, &m_offscreenPass.color.image._allocation, nullptr);

    VkImageViewCreateInfo cview_info = {};
    cview_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    cview_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    cview_info.format = m_swapchainFormat;
    cview_info.subresourceRange = {};
    cview_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    cview_info.subresourceRange.baseMipLevel = 0;
    cview_info.subresourceRange.levelCount = 1;
    cview_info.subresourceRange.baseArrayLayer = 0;
    cview_info.subresourceRange.layerCount = 1;
    cview_info.image = m_offscreenPass.color.image._image;
    vkDestroyImageView(m_device, m_offscreenPass.color.view, nullptr);
    _VK_CHECK(vkCreateImageView(m_device, &cview_info, nullptr, &m_offscreenPass.color.view), "Failed to create ImageView!");


    for (size_t i = 0; i < m_offscreenPass.frameBuffers.size(); i++) {
        vkDestroyFramebuffer(m_device, m_offscreenPass.frameBuffers[i], nullptr);
    }
}

void gns::rendering::Device::InitSyncStructures()
{
    PROFILE_FUNC
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

void gns::rendering::Device::CreateDescriptorSetLayout(VkDescriptorSetLayout* setLayout,
	const VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t bindingCount,
	VkDescriptorSetLayoutCreateFlags flags)
{
    PROFILE_FUNC
    VkDescriptorSetLayoutCreateInfo set2info = {};
    set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2info.flags = flags;
    set2info.bindingCount = bindingCount;
    set2info.pBindings = setLayoutBindings;
    set2info.pNext = nullptr;

    _VK_CHECK(vkCreateDescriptorSetLayout(m_device, &set2info, nullptr, setLayout), " Failed to create DescriptorSetLayout.");
}

void gns::rendering::Device::CreateDescriptorPool()
{
    PROFILE_FUNC
    const std::vector<VkDescriptorPoolSize> sizes =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(sizes.size());
    pool_info.pPoolSizes = sizes.data();

    _VK_CHECK(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool),
        "Failed to create Descriptor Pool!");
}

void gns::rendering::Device::InitGlobalDescriptors(size_t size)
{
    PROFILE_FUNC
    VkDescriptorSetLayoutBinding cameraBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutBinding sceneBind = DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    const std::vector < VkDescriptorSetLayoutBinding> bindings = { cameraBind, sceneBind };
    CreateDescriptorSetLayout(&m_globalSetLayout, bindings.data(), bindings.size());

    const size_t sceneParamBufferSize = m_imageCount * PadUniformBufferSize(size);
    m_sceneParameterBuffer = CreateBuffer(m_allocator, sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    for (uint32_t i = 0; i < m_imageCount; i++)
    {
        constexpr uint32_t cameraDataStructSize = 3*sizeof(glm::mat4);
        m_frames[i]._cameraBuffer = CreateBuffer(m_allocator, cameraDataStructSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        VkDescriptorSetAllocateInfo allocInfo = CreateAllocateInfo(m_descriptorPool, &m_globalSetLayout);
        vkAllocateDescriptorSets(m_device, &allocInfo, &m_frames[i]._globalDescriptor);

        VkDescriptorBufferInfo cameraInfo = CreateBufferInfo(m_frames[i]._cameraBuffer._buffer, cameraDataStructSize);
        VkDescriptorBufferInfo sceneInfo = CreateBufferInfo(m_sceneParameterBuffer._buffer, size);

        std::vector<VkWriteDescriptorSet> setWrites = {
            WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_frames[i]._globalDescriptor, &cameraInfo, 0),
            WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_frames[i]._globalDescriptor, &sceneInfo, 1)
        };
        const size_t writesCount = setWrites.size();
        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writesCount), setWrites.data(), 0, nullptr);
    }
}

void gns::rendering::Device::InitRenderTargetDescriptor()
{
    PROFILE_FUNC
}

void gns::rendering::Device::InitMaterialSetLayouts()
{
    PROFILE_FUNC
}

size_t gns::rendering::Device::PadUniformBufferSize(size_t originalSize)
{
    PROFILE_FUNC
    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = m_gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if (minUboAlignment > 0) {
        alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    return alignedSize;
}

void gns::rendering::Device::EndFrame()
{
    PROFILE_FUNC
    m_imageIndex++;
    m_imageIndex = m_imageIndex % m_imageCount;
}

gns::rendering::FrameData& gns::rendering::Device::GetCurrentFrame()
{
    PROFILE_FUNC
    return m_frames[m_imageIndex];
}

void gns::rendering::Device::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, bool transfer)
{
    PROFILE_FUNC
    VkCommandBuffer cmd = m_uploadContext._commandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
    VkCommandBufferBeginInfo cmdBeginInfo = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    _VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "FailedTo Begin CommandBuffer");
    function(cmd);
    _VK_CHECK(vkEndCommandBuffer(cmd), "Failed To end CommandBuffer");
    VkSubmitInfo submit = SubmitInfo(&cmd);

    _VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext._uploadFence), "Failed To bubmit Command Buffer");

    vkWaitForFences(m_device, 1, &m_uploadContext._uploadFence, VK_TRUE, static_cast<uint64_t>(-1));
    vkResetFences(m_device, 1, &m_uploadContext._uploadFence);
    vkResetCommandPool(m_device, m_uploadContext._commandPool, 0);
}

void gns::rendering::Device::DisposeBuffer(const Buffer& buffer) const
{
    PROFILE_FUNC;
	vkDeviceWaitIdle(m_device);
    vmaDestroyBuffer(m_allocator, buffer._buffer, buffer._allocation);
    LOG_INFO("Buffer Disposed! --Debug id:" << buffer.debugID);
}

uint32_t gns::rendering::Device::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties,
                                               VkBool32* memTypeFound) const
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}
