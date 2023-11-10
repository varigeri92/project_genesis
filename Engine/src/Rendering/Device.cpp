#define VK_USE_PLATFORM_WIN32_KHR
#include "Device.h"

#include <algorithm>
#include <set>

#include "log.h"
#include "vklog.h"
#include "../Window/Window.h"

namespace gns::rendering
{

#pragma region DebugMessenger
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

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
#pragma endregion

	Device::Device(Window* window)
	{
		LOG_INFO("Create Device");
		CreateInstance(window);
		SetupDebugMessenger();
		CreateSurface(window);
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain(window);
	}

	Device::~Device()
	{
		LOG_INFO("Cleanup Device");
		for (const auto imageView : m_swapChainImageViews) {
			vkDestroyImageView(m_device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
		vkDestroyDevice(m_device, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		if (VALIDATION_ENABLED)
			DestroyDebugUtilsMessengerEXT(m_instance, debugMessenger, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void Device::CreateInstance(Window* window)
	{
		if (VALIDATION_ENABLED)
			m_layers.push_back("VK_LAYER_KHRONOS_validation");
		const char* layerName = "";
		if (!CheckLayerSupport())
			LOG_ERROR("Validation Layer not Supported!");

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.apiVersion = VK_API_VERSION_1_2;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pApplicationName = "Vulkan_App";
		appInfo.pEngineName = "Vulkan_Engine";


		const std::vector<const char*> extensionNames = GetRequiredExtensions(window);
		uint32_t extensionCount = static_cast<uint32_t>(extensionNames.size());

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.enabledExtensionCount = extensionCount;
		instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
		instanceCreateInfo.ppEnabledLayerNames = m_layers.data();
		instanceCreateInfo.flags = 0;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.pNext = nullptr;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (VALIDATION_ENABLED) {
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
			instanceCreateInfo.ppEnabledLayerNames = m_layers.data();
			populateDebugMessengerCreateInfo(debugCreateInfo);
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			instanceCreateInfo.enabledLayerCount = 0;
		}

		_VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance), "Failed to create Instance");

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	}

	std::vector<const char*> Device::GetRequiredExtensions(Window* window) const
	{
		uint32_t extensionCount = 0;
		window->GetExtentions(extensionCount, nullptr);
		std::vector<const char*> extensionNames(extensionCount);
		window->GetExtentions(extensionCount, extensionNames.data());

		if (VALIDATION_ENABLED) {
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensionNames;
	}

	void Device::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		_VK_CHECK(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &debugMessenger), "Failed to create Debug messenger");
	}

	void Device::CreateSurface(Window* window)
	{
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = window->hwndHandle;
		createInfo.hinstance = window->hinstance;

		_VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface), "Failed to create Surface");
	}

	void Device::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			LOG_VK_ERROR("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (IsDeviceSuitable(device)) {
				m_physicalDevice = device;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE) {
			LOG_VK_ERROR("failed to find a suitable GPU!");
		}
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
	}

	bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void Device::FindQueueFamilies(VkPhysicalDevice device)
	{
		m_indices = {};
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				m_indices.graphicsFamily = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
			if (presentSupport) {
				m_indices.presentFamily = i;
			}
			i++;
		}
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Device::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}


	VkPresentModeKHR Device::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Device::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window* window)
	{

		int width, height;
		window->GetExtent(width, height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;

	}


	bool Device::IsDeviceSuitable(VkPhysicalDevice device)
	{
		FindQueueFamilies(device);
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return m_indices.IsComplete() && extensionsSupported && swapChainAdequate;
	}

	uint32_t Device::EvaluateDevice(VkPhysicalDevice physical_device)
	{

		return 0;
	}

	bool Device::CheckLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_layers) {
			bool layerFound = false;
			LOG_INFO(layerName);
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void Device::CreateLogicalDevice()
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = {
			m_indices.graphicsFamily.value(),
			m_indices.presentFamily.value()
		};

		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (VALIDATION_ENABLED) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_layers.size());
			createInfo.ppEnabledLayerNames = m_layers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		_VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device),
			"failed to create logical device!");

		vkGetDeviceQueue(m_device, m_indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, m_indices.presentFamily.value(), 0, &m_presentQueue);
	}

	void Device::CreateSwapChain(Window* window)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

		m_imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount) {
			m_imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;

		createInfo.minImageCount = m_imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


		uint32_t queueFamilyIndices[] = { m_indices.graphicsFamily.value(), m_indices.presentFamily.value() };

		if (m_indices.graphicsFamily != m_indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		_VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain),
			"failed to create swap chain!");

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
		m_swapChainImages = std::vector<VkImage>(m_imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, nullptr);
		m_swapChainImages.resize(m_imageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, m_swapChainImages.data());

		CreateImageViews();
	}

	void Device::CreateImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());
		for (size_t i = 0; i < m_swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void Device::RecreateSwapChain(Window* window)
	{
		CreateSwapChain(window);
	}
}
