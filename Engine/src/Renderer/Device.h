#pragma once
#include <functional>
#include <vector>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "Utils/vkb/VkBootstrap.h"
#include "Utils/Buffer.h"
#include "Utils/VulkanImage.h"

namespace gns
{
	class Window;
}
namespace gns::rendering
{
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};

	struct OffscreenPass {
		int32_t width, height;
		std::vector<VkFramebuffer> frameBuffers;
		FrameBufferAttachment color, depth;
		VkSampler sampler;
		VkDescriptorImageInfo descriptor;
		VkRenderPass renderPass;
	};

	struct RenderPassData
	{
		uint32_t bufferWidth;
		uint32_t bufferHeight;
		VkFramebuffer framebuffer;
		FrameBufferAttachment colorAttachment;
		FrameBufferAttachment depthAttachment;
		VkSampler sampler;
	};
	
	typedef struct FrameData {
		VkSemaphore _presentSemaphore;
		VkSemaphore _renderSemaphore;
		VkFence _renderFence;

		VkCommandPool _commandPool;
		VkCommandBuffer _mainCommandBuffer;

		Buffer _cameraBuffer;
		VkDescriptorSet _globalDescriptor;

		Buffer _objectBuffer;
		VkDescriptorSet _objectDescriptor;

	} FrameData;

	struct UploadContext {
		VkFence _uploadFence;
		VkCommandPool _commandPool;
		VkCommandBuffer _commandBuffer;

		void Destroy(VkDevice device);
	};

	class Device
	{
	public:
		Device(Window* window);
		~Device();
		Device(Device& other) = delete;
		Device operator=(Device& other) = delete;

		Window* m_window;

		OffscreenPass m_offscreenPass;

		VmaAllocator m_allocator;
		uint32_t m_imageCount;
		uint32_t m_imageIndex;
		vkb::Instance m_vkb_instance;
		VkInstance m_instance;
		VkSurfaceKHR m_surface;

		vkb::Device m_vkb_device;
		VkPhysicalDevice m_physicalDevice;
		VkPhysicalDeviceProperties m_gpuProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;
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
		UploadContext m_uploadContext;

		VkRenderPass m_renderPass;
		VkRenderPass m_guiPass;
		std::vector<VkFramebuffer> m_frameBuffers;

		//the format for the depth image
		VkFormat m_depthFormat;

		VkDescriptorSetLayout m_globalSetLayout;
		VkDescriptorSetLayout m_objectSetLayout;
		VkDescriptorSetLayout m_RenderTargetLayout;

		VkDescriptorPool m_descriptorPool;
		VkDescriptorPool m_imGuiPool;

		Buffer m_sceneParameterBuffer;
		
		bool InitVulkan();
		void CreateSurface();
		void CreateSwapchain();
		void RebuildSwapchain(int width, int height);
		void CreateCommandPool();
		void CreateRenderTarget(uint32_t width, uint32_t height);
		void InitDefaultRenderPass();
		void InitTextureRenderPass();
		void InitGUIRenderPass();
		void InitFrameBuffers();
		void InitOffscreenFrameBuffers();
		void DestroyOffscreenFrameBuffer() const;
		void InitSyncStructures();
		void CreateDescriptorSetLayout(VkDescriptorSetLayout* setLayout,
			const VkDescriptorSetLayoutBinding* setLayoutBindings, 
			uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags = 0);
		void CreateDescriptorPool();
		void InitGlobalDescriptors(size_t size);
		void InitRenderTargetDescriptor();
		void InitMaterialSetLayouts();
		size_t PadUniformBufferSize(size_t originalSize);
		void EndFrame();
		FrameData& GetCurrentFrame();
		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function,
			bool transfer = true);
		void DisposeBuffer(const Buffer& buffer) const;
		
		
	private:
		uint32_t GetMemoryType(uint32_t typeBits,
			VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	};
}
