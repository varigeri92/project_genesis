#include "Application.h"
#include "Window/Window.h"
#include "Core/Time.h"
#include "Core/Scene.h"
#include "Log.h"
#include "Rendering/RenderSystem.h"
#include "AssetDatabase/AssetLoader.h"
#include "Core/Event.h"
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
Event<void, std::string>* testEvent = new Event<void, std::string>();
void gns::Application::Start(std::function<void()> OnStart)
{
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
		//testEvent->Dispatch("Hello");
		UpdateSystems();
		Render(SceneManager::GetActiveScene());
		Time::EndFrameTime();
	}

	gui->DisposeGUI();
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

