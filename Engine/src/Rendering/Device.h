#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "vkb/VkBootstrap.h"
#include <vector>
#include "Helpers/Buffer.h"


namespace gns
{
	class Window;
}

namespace gns::rendering
{
	struct Mesh;
	struct Material;
	class Renderer;
	class PipelineBuilder;

	typedef struct FrameData {
		VkSemaphore _presentSemaphore;
		VkSemaphore _renderSemaphore;
		VkFence _renderFence;

		VkCommandPool _commandPool;
		VkCommandBuffer _mainCommandBuffer;
	} FrameData;

	class Device
	{
		friend struct Material;
		friend struct Mesh;
		friend class Renderer;
		friend class PipelineBuilder;
	public:
		Device() = delete;
		Device(Window* window);
		~Device();
		Device operator=(const Device& other) = delete;

	private:
		VmaAllocator m_allocator;

		uint32_t m_imageCount;
		uint32_t m_imageIndex;
		vkb::Instance m_vkb_instance;
		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		vkb::Device m_vkb_device;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		VkQueue m_graphicsQueue;
		uint32_t m_graphicsFamilyIndex;
		VkQueue m_transferQueue;
		uint32_t m_transferFamilyIndex;
		VkQueue m_presentQueue;
		uint32_t m_presentFamilyIndex;
		VkQueue m_computeQueue;
		uint32_t m_computeFamilyIndex;


		vkb::Swapchain m_vkb_swapchain;
		VkSwapchainKHR m_swapchain;
		VkFormat m_swapchainFormat;
		std::vector<VkImageView> m_imageViews;

		VkImageView _depthImageView;
		VulkanImage _depthImage;

		std::vector<FrameData> m_frames;

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_frameBuffers;

		//the format for the depth image
		VkFormat m_depthFormat;


		bool InitVulkan(Window* window);
		void CreateSurface(Window* window);
		void CreateSwapchain(Window* window);
		void RebuildSwapchain();

		void CreateCommandPool();

		void InitDefaultRenderPass();

		void InitFrameBuffers();
		void InitSyncStructures();
		void EndFrame();
		FrameData& GetCurrentFrame();
	};
}

