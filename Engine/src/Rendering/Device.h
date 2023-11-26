#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "vkb/VkBootstrap.h"
#include <vector>
#include "Helpers/Buffer.h"

#include "glm/glm.hpp"

namespace gns
{
	class Window;
}

namespace gns::rendering
{
	constexpr size_t MAX_OBJECTS = 1000;
	struct Mesh;
	struct Material;
	class Renderer;
	class PipelineBuilder;

	struct alignas(64) GPUCameraData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewproj;
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

	struct alignas(64) GPUSceneData {
		glm::vec4 fogColor; // w is for exponent
		glm::vec4 fogDistances; //x for min, y for max, zw unused.
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection; //w for sun power
		glm::vec4 sunlightColor;
	};

	struct GPUObjectData {
		glm::mat4 modelMatrix;
	};

	class Device
	{
		friend struct Material;
		friend struct Mesh;
		friend class Renderer;
		friend class PipelineBuilder;
		friend class Buffer;
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
		VkPhysicalDeviceProperties m_gpuProperties;
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

		VkDescriptorSetLayout m_globalSetLayout;
		VkDescriptorSetLayout m_objectSetLayout;
		VkDescriptorPool m_descriptorPool;

		GPUSceneData m_sceneParameters = {};
		Buffer m_sceneParameterBuffer;


		bool InitVulkan(Window* window);
		void CreateSurface(Window* window);
		void CreateSwapchain(Window* window);
		void RebuildSwapchain();

		void CreateCommandPool();

		void InitDefaultRenderPass();

		void InitFrameBuffers();
		void InitSyncStructures();
		void InitDescriptors();
		size_t PadUniformBufferSize(size_t originalSize);
		void EndFrame();
		FrameData& GetCurrentFrame();
	};
}

