#pragma once
#include "Device.h"
#include "Renderer.h"
#include "../SystemsApi/SystemBase.h"
#include "Lights/LightSubsystem.h"

namespace gns
{
	struct Transform;
	struct Camera;

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

	struct GPU_DirectionalLight
	{
		glm::vec4 direction; //w for Intensity
		glm::vec4 color; //w for padding
	};

	struct GPU_PointLight
	{
		glm::vec4 position; //w for range 
		glm::vec4 color; //w for Intensity
	};

	struct GPU_SpotLight
	{
		glm::vec4 position; //w for range
		glm::vec4 direction; //w for Intensity
		glm::vec4 color; //w for angle
	};

	struct alignas(64) SceneLights
	{
		std::vector<GPU_DirectionalLight> directionalLights;
		std::vector<GPU_PointLight> pointLights;
		std::vector<GPU_SpotLight> spotLights;
	};

	struct alignas(64) SceneData
	{
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection; //w for sun power
		glm::vec4 sunlightColor;
		glm::vec4 viewPos;
	};

	class RenderSystem : public gns::SystemBase
	{
	private:
		LightSubsystem lightSubsystem;
		CameraData camData;
		gns::rendering::Device* m_device;
		gns::rendering::Renderer* m_renderer;
		void UpdateMatrix(glm::mat4& matrix, 
			const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale);

		void BatchLights();
		Camera* m_renderCamera;
		Transform* m_transform;

	public:
		GNS_API RenderSystem();
		RenderSystem(RenderSystem& other) = delete;
		GNS_API ~RenderSystem();
		GNS_API void OnCreate() override;
		GNS_API void OnUpdate(float deltaTime) override;
		GNS_API void SetRenderCamera(Camera* renderCamera, Transform* transform);

		rendering::Device* GetDevice() const;
		rendering::Renderer* GetRenderer() const;
		void DisposeShader(rendering::Shader* shader);
		void DisposeTexture(rendering::Texture* texture);
		void DisposeMesh(rendering::MeshData* mesh);
		void DisposeMaterial(rendering::Material* material);
		GNS_API rendering::RenderPass& GetOffscreenPass() const;
		GNS_API void RecreateFrameBuffer(uint32_t width, uint32_t height);
		GNS_API void CreatePipeline(rendering::Shader* shader);
		GNS_API void UploadMesh(rendering::Mesh* mesh);
	};
}
