#pragma once
#include "Device.h"
#include "PipelineBuilder.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "BufferHelper.hpp"
class gns::Window;
using namespace gns::rendering;
namespace gns
{
	struct Mesh;
	namespace core
	{
		struct Material;
	}

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

}

namespace gns::rendering
{
	class Renderer
	{
		friend class Window;
	public:
		Renderer(Window* window);
		~Renderer();
		Renderer(const Renderer& obj) = delete;
		Renderer operator= (const Renderer& other) = delete;

		void CreateRenderer(Material& material);
		void CreateTexture(Texture& texture);
		bool BeginFrame(uint32_t& imageIndex);
		void DrawFrame();
		void BeginDraw( uint32_t imageIndex);
		void DrawMesh(Mesh* mesh, uint32_t imageIndex, Material& material);
		void EndDraw(uint32_t imageIndex);
		void EndFrame(uint32_t& imageIndex);
		void CreateVertexBuffer(Mesh* mesh);
		void CreateIndexBuffer(Mesh* mesh);
		void UpdateUniformBuffer(uint32_t currentImage, UniformBufferObject& ubo);
		void DeleteMesh(Mesh* mesh);

	private:
		inline static bool m_frameBufferResized = false;
		inline static bool m_frameBufferMinimized = false;

		Window* m_window;
		uint32_t m_currentFrame=0;
		Device* m_device;
		PipelineBuilder* m_pipelineBuilder;

		std::vector<VkFramebuffer> m_frameBuffers;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;

		//VkImage m_textureImage;
		//VkDeviceMemory m_textureImageMemory;
		//VkImageView m_textureImageView;
		//VkSampler m_textureSampler;


		void CreateFranmeBuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandbuffer,uint32_t imageIndex, Mesh* mesh, Material& material);
		void CreateSyncObjects();
		void RecreateSwapChain();
		void CleanupSwapChain();

		
		void CreateTexture_Internal();
		void CreateTexture_Internal(Texture& texture);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void CreateTextureImageView(Texture& texture);
		void CreateTextureSampler(Texture& texture);
	};
}

