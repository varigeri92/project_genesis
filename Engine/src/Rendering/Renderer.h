#pragma once
#include <memory>
#include <string>

#include "rendererfwd.h"
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "DataObjects/Mesh.h"
#include <unordered_map>
#include "DataObjects/Material.h"
#include "DataObjects/DisposableVkObject.h"

namespace gns
{
	class Application;
}

namespace gns::rendering
{
	struct GPUSceneData;
	struct GPUCameraData;
	constexpr unsigned int FRAME_OVERLAP = 2;
	class Renderer
	{
		friend class gns::Application;
		struct MeshPushConstants {
			glm::mat4 model_matrix;
			glm::mat4 camera_matrix;
		};
	public:
		Renderer(Window* window, size_t buffersize);
		~Renderer();
		Renderer(Renderer& other);
		Renderer operator=(Renderer& other) = delete;
		GEN_API bool BeginFrame(uint32_t& swapchainImageIndex);
		GEN_API void BeginRenderPass(uint32_t& swapchainImageIndex, bool gui);
		GEN_API void EndRenderPass(uint32_t& swapchainImageIndex);
		GEN_API void Draw(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, 
			int index, size_t uniformSize);

		GEN_API void EndFrame(uint32_t& swapchainImageIndex);
		GEN_API void UploadMesh(Mesh* mesh);
		GEN_API void CreatePipelineForMaterial(std::shared_ptr<Shader> shader);
		
		GEN_API void DisposeObject(std::shared_ptr<IDisposeable> object);
	private:

		VkClearValue m_clearValue;
		VkClearValue m_depthClear;
		VkClearValue m_clearValues[2];
		VkRenderPassBeginInfo m_mainRPInfo;

		bool m_framebufferResized = false;
		bool m_pipelineBound = false;
		std::vector<std::shared_ptr<IDisposeable>> _disposeQueue;
		uint32_t m_frameNumber{ 0 };
		float m_rotation{ 0 };
		float m_flash{ 0 };
		float m_rotationSpeed{25};
		Device* m_device;
		uint32_t m_framesInFlight;

		VkShaderModule CreateShaderModule(std::vector<uint32_t> buffer);
		void CreateVertexBuffers(Mesh* mesh);
		void CreateIndexBuffers(Mesh* mesh);
		std::shared_ptr<Material> m_material_ptr;
		//MOVE OUT OF HERE:
	public:
		GEN_API void UpdatePushConstant(
			glm::mat4 modelMatrix, 
			std::shared_ptr<Material> material);

		GEN_API void UpdateGlobalUbo(GPUCameraData src_bufferData);
		GEN_API void UpdateSceneDataUbo(const GPUSceneData& data);
		GEN_API void UpdateSceneDataUbo(const void* data, size_t size);
		GEN_API void UpdateObjectData(void* src_data, size_t size);
		GEN_API void UpdateMaterialUniformBuffer(void* src_data, size_t size, Material* material);
		GEN_API void CreateTextureResources(std::shared_ptr<Texture> texture);
	};
}
