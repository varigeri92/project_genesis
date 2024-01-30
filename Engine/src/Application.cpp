#include "Application.h"
#include "Window/Window.h"
#include "Core/Time.h"
#include "AssetDatabase/Guid.h"
#include "Core/Scene.h"
#include "Log.h"
#include "AssetDatabase/AssetLoader.h"
#include "ECS/Entity.h"
#include "GUI/GUI.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Rendering/Renderer.h";
#include "Rendering/Device.h";
#include "Rendering/DataObjects/Texture.h"


GPUSceneData sceneData = {};

gns::Application::Application(std::string assetsPath)
{
	AssetLoader::SetPaths(assetsPath);
	m_window = new Window();
	m_close = false;
	m_renderer = new rendering::Renderer(m_window);
	/*Testing Randoms
	 
	#pragma region Random_Test
	gns::core::guid _guid = gns::core::Guid::GetNewGuid();
	LOG_INFO(_guid);

	uint16_t u16 = Random::Get(0, 15);
	LOG_INFO(u16);
	uint32_t u32 = Random::Get(0, 15);
	LOG_INFO(u32);
	uint64_t u64 = Random::Get(0, 15);
	LOG_INFO(u64);

	int64_t i16 = Random::Get(-100, -1);
	LOG_INFO(i16);
	int64_t i32 = Random::Get(-100, 100);
	LOG_INFO(i32);
	int64_t i64 = Random::Get(0, 10000);
	LOG_INFO(i64);

	float f = Random::Get(0.f, 10000.f);
	LOG_INFO(f);
	double d = Random::Get(-10000.0, 10000.0);
	LOG_INFO(d);
	#pragma endregion
	 */

	sceneData.ambientColor = { 0.299f,0.145f,0.145f,0.150f };
	sceneData.sunlightColor = {1.f, 1.f, 1.f, 1.f};
	sceneData.sunlightDirection = { 1.f, 1.f, 0.f , 0.f};
}

gns::Application::~Application()
{
	delete(m_renderer);
	delete(m_window);
}

void gns::Application::Run()
{
	//Load Suzan:
	std::shared_ptr<Mesh> suzan_mesh = AssetLoader::LoadMesh(R"(Meshes\OtherModels\Rei\Rei.obj)");
	m_renderer->UploadMesh(suzan_mesh.get());

	std::shared_ptr<Mesh> groundPlaneMesh = AssetLoader::LoadMesh(R"(Meshes\plane.obj)");
	m_renderer->UploadMesh(groundPlaneMesh.get());

	std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("blinphong.vert.spv","blinphong.frag.spv");
	std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(defaultShader, "Rei_Material");
	m_renderer->CreatePipelineForMaterial(defaultMaterial);

	/* 
	//load MC:
	std::shared_ptr<Mesh> mc_mesh = AssetLoader::LoadMesh( R"(Meshes\lost_empire.obj)");
	m_renderer->UploadMesh(mc_mesh.get());
	std::shared_ptr<Shader> mc_shader = std::make_shared<Shader>(R"(Shaders\tri_mesh.vert.spv)",R"(Shaders\textured_lit.frag.spv)");
	std::shared_ptr<Material> mc_material = std::make_shared<Material>(mc_shader, "mc_material");
	mc_material->texture = std::make_shared<Texture>(R"(Textures\lost_empire-RGBA.png)");
	mc_material->texture->Create(m_renderer->m_device);
	m_renderer->CreatePipelineForMaterial(mc_material);
	mc_material->texture->Apply(m_renderer->m_device);
	*/


	//Entity Creation:
	//Create Scene
	std::shared_ptr<Scene> scene = std::make_shared<Scene>("Default Scene");
	/* 
	Entity mc_entity = scene->CreateEntity("MC_Lost_Empire", scene.get());
	mc_entity.AddComponet<MeshComponent>(mc_mesh);
	mc_entity.AddComponet<MaterialComponent>(mc_material);
	mc_entity.GetTransform().matrix = glm::mat4{ 1 };
	*/

	Entity suzan = scene->CreateEntity("Ayanami_Rei", scene.get());
	suzan.AddComponet<RendererComponent>(suzan_mesh, defaultMaterial);
	suzan.GetComponent<Transform>().matrix = glm::mat4{ 1 };

	Entity floor = scene->CreateEntity("GroundFloor", scene.get());
	floor.AddComponet<RendererComponent>(groundPlaneMesh, defaultMaterial);
	floor.GetComponent<Transform>().matrix = glm::mat4{ 1 };

	Entity sceneCamera_entity = scene->CreateEntity("SceneCamera", scene.get());
	Transform& cameraTransform = sceneCamera_entity.GetTransform();
	cameraTransform.position = { 0.f,-1.f,-3.f };
	CameraComponent& sceneCamera = sceneCamera_entity.AddComponet<CameraComponent>(0.01f, 1000.f, 60.f, 1700, 900, cameraTransform);
	CameraSystem cameraSystem = { cameraTransform, sceneCamera };
	gui = new GUI{ m_renderer->m_device, m_window };

	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();
		UpdateSystems();
		cameraSystem.UpdateCamera();
		UpdateLate();
		gui->BeginGUI();
		gui->DrawGUI();
		//  Scene Data Editor:
		ImGui::Begin("Scene Data Editor");

		ImGui::ColorEdit3("Ambient Color", (float*)&sceneData.ambientColor);
		ImGui::DragFloat("Ambient Intensity", &sceneData.ambientColor.w, 0.01f, 0);

		ImGui::ColorEdit3("Light Color", (float*)&sceneData.sunlightColor);
		ImGui::DragFloat("Light Intensity", &sceneData.sunlightColor.w, 0.001f, 0);

		ImGui::DragFloat3("light Direction", (float*)&sceneData.sunlightDirection, 0.001f, -1.0f, 1.0f);

		ImGui::End();
		Render(scene);
		Time::EndFrameTime();
	}

	gui->DisposeGUI();
	m_renderer->DisposeObject(defaultMaterial);
	m_renderer->DisposeObject(suzan_mesh);
	m_renderer->DisposeObject(groundPlaneMesh);
}

void gns::Application::CloseApplication()
{
	m_close = true;
}

void gns::Application::Render(std::shared_ptr<Scene> scene)
{
	GPUCameraData camData = {};
	auto cameraView = scene->registry.view<Transform, CameraComponent>();
	for (auto [entt, transform, camera] : cameraView.each())
	{
		camData.view = camera.view;
		camData.proj = camera.projection;
		camData.viewproj = camera.camera_matrix; //camera.projection * camera.view;
	}

	uint32_t swapchainImageIndex;
	if(m_renderer->BeginFrame(swapchainImageIndex))
	{
		m_renderer->BeginRenderPass(swapchainImageIndex, false);
		m_renderer->UpdateGlobalUbo(camData);
		m_renderer->UpdateSceneDataUbo(sceneData);
		auto entityView = scene->registry.view<Transform, RendererComponent, EntityComponent>();
		for (auto [entt, transform, rendererComponent, entity] : entityView.each())
		{
			m_renderer->UpdatePushConstant(transform.matrix, rendererComponent.material);
			m_renderer->Draw(rendererComponent.mesh, rendererComponent.material, swapchainImageIndex);
		}
		m_renderer->EndRenderPass(swapchainImageIndex);
		m_renderer->BeginRenderPass(swapchainImageIndex, true);
		gui->EndGUI();
		m_renderer->EndRenderPass(swapchainImageIndex);
		m_renderer->EndFrame(swapchainImageIndex);
	}
}


void gns::Application::UpdateSystems()
{

}

void gns::Application::UpdateLate()
{
		
}

void gns::Application::HandleEvents()
{
	m_close = m_window->PollEvents();
}

