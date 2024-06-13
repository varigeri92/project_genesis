#pragma once
#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>

namespace gns::rendering
{
	class Shader;
	class Device;
	class DrawData;
	struct MeshData;
	struct Material;
	class Texture;

	struct MeshPushConstants {
		glm::mat4 model_matrix;
		glm::mat4 camera_matrix;
	};
	class Renderer
	{
	public:
		Renderer(Device* device);
		~Renderer();
		Renderer(Renderer& other) = delete;
		Renderer operator=(Renderer& other) = delete;

		bool BeginFrame(uint32_t& swapchainImageIndex);
		void BeginRenderPass(uint32_t& swapchainImageIndex, bool gui);
		void BeginTextureRenderPass(uint32_t& swapchainImageIndex);
		void BeginGuiRenderPass(uint32_t& swapchainImageIndex);
		void Draw(MeshData* mesh, Material* material, int index, size_t uniformSize);
		void EndRenderPass(uint32_t& swapchainImageIndex);
		void EndFrame(uint32_t& swapchainImageIndex);


		void UploadMesh(MeshData* mesh);
		void CreatePipeline(Shader* shader);
		void UpdatePushConstant(glm::mat4 modelMatrix, Material* material);
		void UpdateGlobalUbo(void* src_bufferData, size_t size);
		void UpdateMaterialUbo(Material* material);
		void UpdateSceneDataUbo(const void* data, size_t size);
		void UpdateObjectData(void* src_data, size_t size);
		void UpdateMaterialUniformBuffer(void* src_data, size_t size, const std::shared_ptr<Material>& material);
		void CreateTextureResources(const std::shared_ptr<Texture>& texture);


	private:
		VkShaderModule CreateShaderModule(std::vector<uint32_t> buffer);
		void CreateVertexBuffers(MeshData* mesh);
		void CreateIndexBuffers(MeshData* mesh);
		void CreateSetLayoutBindings(Shader* shader);

		uint32_t m_framesInFlight;
		uint32_t m_frameNumber{ 0 };

		Device* m_device;
		VkClearValue m_clearValues[2];
		VkRenderPassBeginInfo m_mainRPInfo;

		bool m_frameBufferResized = false;
		bool m_pipelineBound = false;

		Material* m_currentMaterial;
	};
}
