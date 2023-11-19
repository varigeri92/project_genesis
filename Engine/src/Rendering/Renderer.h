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

namespace gns::rendering
{
	constexpr unsigned int FRAME_OVERLAP = 2;
	class Renderer
	{
		struct MeshPushConstants {
			glm::vec4 data;
			glm::mat4 render_matrix;
		};
	public:
		Renderer(Window* window);
		~Renderer();
		Renderer operator=(Renderer& other) = delete;
		void BeginFrame(uint32_t& swapchainImageIndex);
		void Draw(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
		void EndFrame(uint32_t& swapchainImageIndex);
		void UploadMesh(Mesh* mesh);
		void CreatePipelineForMaterial(std::shared_ptr<Material> material);
		
		void DisposeObject(std::shared_ptr<Disposeable> object);
	private:
		std::vector<std::shared_ptr<Disposeable>> _disposeQueue;
		uint32_t m_frameNumber{ 0 };
		float m_rotation{ 0 };
		float m_flash{ 0 };
		float m_rotationSpeed{25};
		Device* m_device;
		uint32_t m_framesInFlight;

		VkShaderModule CreateShaderModule(std::vector<uint32_t> buffer);
		//MOVE OUT OF HERE:
	public:
		void UpdatePushConstant( 
			glm::mat4 modelMatrix, 
			std::shared_ptr<Material> material);
	};
}
