#include "Application.h"
#include "Window/Window.h"
#include "Core/Time.h"
#include "Core/Scene.h"
#include "Log.h"
#include "Rendering/RenderSystem.h"
#include "AssetDatabase/AssetLoader.h"
#include "ECS/Entity.h"
#include "GUI/GUI.h"

#include "Rendering/Renderer.h"
#include "Rendering/Device.h"
#include "Rendering/DataObjects/Texture.h"
#include "Core/SceneManager.h"
#include "ECS/SystemsApi.h"

size_t buffersize;
gns::Application::Application(std::string assetsPath)
{
	buffersize = sizeof(Scene::GPUSceneData);
	AssetLoader::SetPaths(assetsPath);
	m_window = new Window();
	m_close = false;
	RenderSystem::S_Renderer = new rendering::Renderer(m_window, buffersize);
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
}

gns::Application::~Application()
{
	delete(RenderSystem::S_Renderer);
	delete(m_window);
}

void gns::Application::Start(std::function<void()> OnStart)
{
	/*
	std::shared_ptr<Mesh> suzan_mesh = AssetLoader::LoadMesh(R"(Meshes\OtherModels\Rei\Rei.obj)");
	RenderSystem::S_Renderer->UploadMesh(suzan_mesh.get());

	std::shared_ptr<Mesh> groundPlaneMesh = AssetLoader::LoadMesh(R"(Meshes\plane.obj)");
	RenderSystem::S_Renderer->UploadMesh(groundPlaneMesh.get());

	std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("blinphong.vert.spv", "blinphong.frag.spv");
	LOG_INFO(defaultShader->GetGuid());
	RenderSystem::S_Renderer->CreatePipelineForMaterial(defaultShader);
	std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(defaultShader, "Rei_Material");
	LOG_INFO(defaultMaterial->GetGuid());
	defaultMaterial->m_texture = std::make_shared<Texture>(R"(Textures\lost_empire-RGBA.png)");
	defaultMaterial->m_texture->Apply();
	std::shared_ptr<Texture> icon = std::make_shared<Texture>(R"(Resources\Icons\icon_file.png)");
	icon->Apply();

	//Entity Creation:
	//Create Scene
	Scene* scene = SceneManager::CreateScene("Default Scene created With the manager!");
	//scene = std::make_shared<Scene>("Default Scene");
	

	Entity suzan = scene->CreateEntity("Ayanami_Rei", scene);
	suzan.AddComponet<RendererComponent>(suzan_mesh, defaultMaterial);
	suzan.GetComponent<Transform>().matrix = glm::mat4{ 1 };

	Entity floor = scene->CreateEntity("GroundFloor", scene);
	floor.AddComponet<RendererComponent>(groundPlaneMesh, defaultMaterial);
	floor.GetComponent<Transform>().matrix = glm::mat4{ 1 };

	Entity sceneCamera_entity = scene->CreateEntity("SceneCamera", scene);
	Transform& cameraTransform = sceneCamera_entity.GetTransform();
	cameraTransform.position = { 0.f,-1.f,-3.f };
	CameraComponent& sceneCamera = sceneCamera_entity.AddComponet<CameraComponent>(0.01f, 1000.f, 60.f, 1700, 900, cameraTransform);
	cameraSystem = new CameraSystem{ cameraTransform, sceneCamera };
	gui = new GUI{ RenderSystem::S_Renderer->m_device, m_window };
	*/

	OnStart();
	gui = new GUI{ RenderSystem::S_Renderer->m_device, m_window };

}

void gns::Application::Run(std::function<void()> OnUpdate)
{
	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();
		gui->BeginGUI();
		gui->DrawGUI();

		OnUpdate();

		UpdateSystems();
		/*
		ImVec2 size = ImVec2(40.0f, 40.0f);                         // Size of the image we want to make visible
		ImVec2 uv0 = ImVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
		ImVec2 uv1 = ImVec2(1.f,1.f);
		ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGui::Begin("Vulkan Texture Test");
		ImGui::Text("pointer = %p", icon->descriptorSet);
		ImGui::Text("size = %d x %d", icon->width, icon->height);
		ImGui::Image(
			(ImTextureID)defaultMaterial->m_texture->descriptorSet, 
			ImVec2(defaultMaterial->m_texture->width, 
				defaultMaterial->m_texture->height)
		);
		if (ImGui::ImageButton("btn", (ImTextureID)icon->descriptorSet, size, uv0, uv1, bg_col, tint_col))
		{
			LOG_INFO("Button Pressed!");
		}
		ImGui::End();
		*/

		Render(SceneManager::GetActiveScene());
		Time::EndFrameTime();
	}

	gui->DisposeGUI();
	/*
	RenderSystem::S_Renderer->DisposeObject(defaultMaterial);
	RenderSystem::S_Renderer->DisposeObject(suzan_mesh);
	RenderSystem::S_Renderer->DisposeObject(groundPlaneMesh);
	*/
}

void gns::Application::CloseApplication()
{
	m_close = true;
}

void gns::Application::Render(Scene* scene)
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
	if(RenderSystem::S_Renderer->BeginFrame(swapchainImageIndex))
	{
		RenderSystem::S_Renderer->BeginRenderPass(swapchainImageIndex, false);
		RenderSystem::S_Renderer->UpdateGlobalUbo(camData);
		RenderSystem::S_Renderer->UpdateSceneDataUbo((void*)&scene->sceneData, sizeof(scene->sceneData));
		auto entityView = scene->registry.view<Transform, RendererComponent, EntityComponent>();
		for (auto [entt, transform, rendererComponent, entity] : entityView.each())
		{
			RenderSystem::S_Renderer->UpdatePushConstant(transform.matrix, rendererComponent.material);
			RenderSystem::S_Renderer->Draw(rendererComponent.mesh, rendererComponent.material, swapchainImageIndex, buffersize);
		}
		RenderSystem::S_Renderer->EndRenderPass(swapchainImageIndex);
		RenderSystem::S_Renderer->BeginRenderPass(swapchainImageIndex, true);
		gui->EndGUI();
		RenderSystem::S_Renderer->EndRenderPass(swapchainImageIndex);
		RenderSystem::S_Renderer->EndFrame(swapchainImageIndex);
	}
}


void gns::Application::UpdateSystems()
{
	gns::core::SystemsApi::UpdateSystems();
}

void gns::Application::UpdateLate()
{
		
}

void gns::Application::HandleEvents()
{
	m_close = m_window->PollEvents();
}

