#pragma once
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include "vkb/VkBootstrap.h"
#include <vector>
#include "Helpers/Buffer.h"
#include "glm/glm.hpp"
#include <functional>

namespace gns
{
	class GUI;
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

	/* 
	struct alignas(64) GPUSceneData {
		glm::vec4 fogColor; // w is for exponent
		glm::vec4 fogDistances; //x for min, y for max, zw unused.
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection; //w for sun power
		glm::vec4 sunlightColor;
	};
	*/

	struct GPUObjectData {
		glm::mat4 modelMatrix;
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
		friend struct Material;
		friend struct Mesh;
		friend class Renderer;
		friend class PipelineBuilder;
		friend class Buffer;
		friend class Texture;
		friend class gns::GUI;
	public:
		Device() = delete;
		Device(Window* window);
		~Device();
		Device operator=(const Device& other) = delete;

	private:
		Window* m_window;
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
		UploadContext m_uploadContext;

		VkRenderPass m_renderPass;
		VkRenderPass m_guiPass;
		std::vector<VkFramebuffer> m_frameBuffers;

		//the format for the depth image
		VkFormat m_depthFormat;

		VkDescriptorSetLayout m_globalSetLayout;
		VkDescriptorSetLayout m_objectSetLayout;
		VkDescriptorSetLayout m_singleTextureSetLayout;
		VkDescriptorPool m_descriptorPool;
		VkDescriptorPool m_imGuiPool;

		//GPUSceneData m_sceneParameters = {};
		Buffer m_sceneParameterBuffer;


		bool InitVulkan();
		void CreateSurface();
		void CreateSwapchain();
		void RebuildSwapchain();

		void CreateCommandPool();

		void InitDefaultRenderPass();
		void InitGUIRenderPass();

		void InitFrameBuffers();
		void InitSyncStructures();
		void InitDescriptors(size_t size);
		size_t PadUniformBufferSize(size_t originalSize);
		void EndFrame();
		FrameData& GetCurrentFrame();

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function, bool transfer = true);
	};
}

