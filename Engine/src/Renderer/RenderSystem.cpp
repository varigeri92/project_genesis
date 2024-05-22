#include "gnspch.h"
#include "RenderSystem.h"
#include "Log.h"
#include "Shader.h"
#include "../Gui/GuiSystem.h"
#include "../SystemsApi/ComponentLibrary.h"


static  void printMat4(const glm::mat4& matrix, std::string label) {
	std::cout << std::fixed << std::setprecision(6); // Set precision for floating-point numbers

	// Determine the width needed for each cell
	int cellWidth = 10; // 8 digits + 2 spaces for padding

	// Print matrix values in a table-like structure
	std::cout << label << "\n";
	std::cout << "+------------+------------+------------+------------+" << "\n";
	for (int i = 0; i < 4; ++i) {
		std::cout << "|";
		for (int j = 0; j < 4; ++j) {
			std::cout << std::setw(cellWidth) << matrix[i][j] << " ";
		}
		std::cout << "|" << "\n";
	}
	std::cout << "+------------+------------+------------+------------+" << std::endl;
}

static void printVec3(const glm::vec3& vec) {
	std::cout << std::fixed << std::setprecision(6); // Set precision for floating-point numbers

	// Determine the width needed for each cell
	int cellWidth = 10; // 8 digits + 2 spaces for padding

	// Print vector values in a table-like structure
	std::cout << "+------------+------------+------------+" << std::endl;
	std::cout << "|";
	for (int i = 0; i < 3; ++i) {
		std::cout << std::setw(cellWidth) << vec[i] << " ";
	}
	std::cout << "|" << std::endl;
	std::cout << "+------------+------------+------------+" << std::endl;
}

gns::RenderSystem::RenderSystem(Window* window) : SystemBase(), sceneData()
{
	PROFILE_FUNC
	m_device = new rendering::Device(window);
	m_device->InitGlobalDescriptors(sizeof(SceneData));
	m_renderer = new gns::rendering::Renderer(m_device);

	sceneData.ambientColor = { 1.f,1.f,1.f,0.050f };
	sceneData.sunlightColor = { 1.f, 1.f, 1.f, 1.f };
	sceneData.sunlightDirection = { 1.f, 1.f, 0.f , 0.f };
}
gns::RenderSystem::~RenderSystem()
{
	PROFILE_FUNC
	delete m_renderer;
};

void gns::RenderSystem::OnCreate()
{
	PROFILE_FUNC
}

void gns::RenderSystem::OnUpdate(float deltaTime)
{
	PROFILE_FUNC

	
	auto cameraView = SystemsAPI::GetRegistry().view<Transform, Camera>();
	for (auto [entt, transform, camera] : cameraView.each())
	{
		camData.view = camera.view;
		camData.proj = camera.projection;
		camData.viewproj = camera.camera_matrix;
		/* 
		printMat4(camera.view, "View");
		printMat4(camera.projection, "Projection");
		printMat4(camera.camera_matrix, "view projection");
		*/
	}
	uint32_t swapchainImageIndex;
	if (m_renderer->BeginFrame(swapchainImageIndex))
	{
		m_renderer->BeginTextureRenderPass(swapchainImageIndex);
		m_renderer->UpdateGlobalUbo(&camData, sizeof(CameraData));
		m_renderer->UpdateSceneDataUbo(&sceneData, sizeof(SceneData));
		auto entityView = SystemsAPI::GetRegistry().view<Transform, RendererComponent, EntityComponent>();
		for (auto [entt, transform, rendererComponent, entity] : entityView.each())
		{
			transform.UpdateMatrix();
			m_renderer->UpdatePushConstant(transform.matrix, rendererComponent.m_materials[0]);
			for (size_t i = 0; i < rendererComponent.m_mesh->m_subMeshes.size(); i++)
			{
				m_renderer->Draw(rendererComponent.m_mesh->m_subMeshes[i], rendererComponent.m_materials[0], swapchainImageIndex, sizeof(SceneData));
			}
		}
		m_renderer->EndRenderPass(swapchainImageIndex);
		m_renderer->BeginRenderPass(swapchainImageIndex, true);
		gns::gui::GuiSystem::PresentGui(m_device->GetCurrentFrame()._mainCommandBuffer);
		m_renderer->EndRenderPass(swapchainImageIndex);
		m_renderer->EndFrame(swapchainImageIndex);
	}
}


gns::rendering::Device* gns::RenderSystem::GetDevice() const
{
	return m_device;
}

gns::rendering::Renderer* gns::RenderSystem::GetRenderer() const
{
	return m_renderer;
}

void gns::RenderSystem::DisposeShader(rendering::Shader* shader)
{
	if (shader->shaderSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(m_device->m_device, shader->shaderSetLayout, nullptr);
		shader->shaderSetLayout = VK_NULL_HANDLE;
	}
	if (shader->pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device->m_device, shader->pipelineLayout, nullptr);
		shader->pipelineLayout = VK_NULL_HANDLE;
	}
	if(shader->pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device->m_device, shader->pipeline, nullptr);
		shader->pipeline = VK_NULL_HANDLE;
	}
}


void gns::RenderSystem::DisposeMaterial(rendering::Material* material)
{
	//m_device->DisposeBuffer(material->m_uniformBuffer);
}


gns::rendering::OffscreenPass& gns::RenderSystem::GetOffscreenPass() const
{
	return m_device->m_offscreenPass;
}

void gns::RenderSystem::RecreateFrameBuffer(uint32_t width, uint32_t height)
{
	m_device->DestroyOffscreenFrameBuffer();
	m_device->CreateRenderTarget(width, height);
	m_device->InitTextureRenderPass();
	m_device->InitOffscreenFrameBuffers();
}

void gns::RenderSystem::CreatePipeline(rendering::Shader* shader)
{
	m_renderer->CreatePipeline(shader);
}

void gns::RenderSystem::UploadMesh(rendering::Mesh* mesh)
{
	if (mesh->loaded) return;
	for (size_t i = 0; i < mesh->m_subMeshes.size(); i++)
	{
		m_renderer->UploadMesh(mesh->m_subMeshes[i]);
	}
	mesh->loaded = true;
}
