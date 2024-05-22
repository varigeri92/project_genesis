#pragma once
#include "Device.h"
#include "Renderer.h"
#include "../SystemsApi/SystemBase.h"

namespace gns
{
	namespace rendering
	{
		struct Mesh;
	}

	class Window;
}

namespace gns
{
	struct alignas(64) CameraData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewproj;
	};

	struct alignas(64) SceneData {
		glm::vec4 fogColor; // w is for exponent
		glm::vec4 fogDistances; //x for min, y for max, zw unused.
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection; //w for sun power
		glm::vec4 sunlightColor;
	};

	class RenderSystem : public gns::SystemBase
	{
	private:
		SceneData sceneData;
		CameraData camData;
		gns::rendering::Device* m_device;
		gns::rendering::Renderer* m_renderer;
		void UpdateMatrix(glm::mat4& matrix, 
			const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale);
	public:
		GNS_API RenderSystem(Window* window);
		RenderSystem(RenderSystem& other) = delete;
		GNS_API ~RenderSystem();
		GNS_API void OnCreate() override;
		GNS_API void OnUpdate(float deltaTime) override;

		rendering::Device* GetDevice() const;
		rendering::Renderer* GetRenderer() const;
		void DisposeShader(rendering::Shader* shader);
		void DisposeTexture(rendering::Texture* texture);
		void DisposeMesh(rendering::MeshData* mesh);
		void DisposeMaterial(rendering::Material* material);
		GNS_API rendering::OffscreenPass& GetOffscreenPass() const;
		GNS_API void RecreateFrameBuffer(uint32_t width, uint32_t height);
		GNS_API void CreatePipeline(rendering::Shader* shader);
		GNS_API void UploadMesh(rendering::Mesh* mesh);
	};
}
