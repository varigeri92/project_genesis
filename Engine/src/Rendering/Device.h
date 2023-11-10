#pragma once
#include <optional>
#include <vulkan/vulkan.h>
#include <vector>

namespace gns
{
	class Window;
}

namespace gns::rendering
{
	class Renderer;
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool IsComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	class Device
	{
		friend class Renderer;
	public:
		Device() = delete;
		Device(Window* window);
		~Device();
		Device operator=(const Device& other) = delete;

	private:
		
		bool VALIDATION_ENABLED = true;
		VkDebugUtilsMessengerEXT debugMessenger;

		std::vector<const char*> m_layers = {};
		std::vector<const char*> m_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkInstance m_instance;

		VkSurfaceKHR m_surface;

		VkPhysicalDevice m_physicalDevice;
		VkPhysicalDeviceProperties m_deviceProperties{};
		VkDevice m_device;

		QueueFamilyIndices m_indices;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		uint32_t m_imageCount;
		VkSwapchainKHR  m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;

		void CreateInstance(Window* window);
		std::vector<const char*> GetRequiredExtensions(Window* window) const;
		void SetupDebugMessenger();
		void CreateSurface(Window* window);

		void PickPhysicalDevice();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window* window);

		bool IsDeviceSuitable(VkPhysicalDevice device);
		uint32_t EvaluateDevice(VkPhysicalDevice physical_device);

		bool CheckLayerSupport();

		void CreateLogicalDevice();
		void CreateSwapChain(Window* window);
		void CreateImageViews();
		void RecreateSwapChain(Window* window);
	};
}

