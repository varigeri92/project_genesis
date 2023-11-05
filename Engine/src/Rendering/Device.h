#pragma once
#include "vulkan/vulkan.h"
#include  <vector>
#include <optional>

#include "Log.h"

//Forward declaration:
namespace gns
{
	class Window;
}
//End Forward declaration
namespace gns::rendering
{
	class Device
	{
		friend class Renderer;
		friend class BufferHelper;
		friend class PipelineBuilder;
		friend class VulkanImage;
	private:
		const bool enableValidationLayers = true;

		const std::vector<const char*> m_layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

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

	public:
		Device(gns::Window* window);
		~Device();
		Device(const Device& obj)= delete;
		Device operator= (const Device& other) = delete;


		VkDevice GetDevice() const { return m_device; }
		VkExtent2D GetSwapchainExtent() const { return m_swapChainExtent; }
		VkFormat GetImageFormat() const { return m_swapChainImageFormat; }

	private:
		VkPhysicalDeviceProperties m_deviceProperties{};
		VkDebugUtilsMessengerEXT debugMessenger;

		VkInstance m_instance;
		VkPhysicalDevice m_physDevice;
		VkDevice m_device;

		QueueFamilyIndices m_indices;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		VkSurfaceKHR m_surface;

		VkSwapchainKHR  m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;




		void CreateInstance(gns::Window* window);
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions(gns::Window* window);
		void SetupDebugMessenger();
		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void FindQueueFamilies(VkPhysicalDevice device);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateLogicalDevice();
		void CreateSurface(gns::Window* window);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window* window);
		void CreateSwapChain(Window* window);
		void CreateImageViews();

		void RecreateSwapChain(Window* window);
	};
}