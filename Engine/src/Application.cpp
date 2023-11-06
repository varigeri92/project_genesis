#include "Application.h"

#include "Input.h"
#include "Rendering/Renderer.h"
#include "Window/Window.h"
#include "mesh.h"
#include "Core/Time.h"
#include "AssetDatabase/Guid.h"
#include "AssetDatabase/AssetLoader.h"
#include "Rendering/Material.h"
#include "InputDefines.h"
#include "Core/Scene.h"
#include "ECS/ECSManager.h"
#include "Camera.h"


#include "Rendering/PipelineBuilder.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Components.h";

gns::Application::Application()
{
	m_window = new Window();
	m_close = false;
	m_renderer = new rendering::Renderer(m_window);

	/*Testing Randoms*/
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
}

gns::Application::~Application()
{
	delete(m_renderer);
	delete(m_window);
}

void gns::Application::Run()
{
	std::shared_ptr<Texture> vikingRoomTexture = std::make_shared<Texture>(R"(D:\project_genesis\Engine\Assets\Textures\viking_room.png)");
	std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>(R"(D:\project_genesis\Engine\Assets\Textures\PrototypeGridTexture.jpg)");
	std::shared_ptr<gns::Mesh> planeMesh = AssetLoader::LoadMesh(R"(D:\project_genesis\Engine\Assets\Meshes\plane.obj)");
	std::shared_ptr<gns::Mesh> vikingRoomMesh = AssetLoader::LoadMesh(R"(D:\project_genesis\Engine\Assets\Meshes\viking_room.obj)");
	Material vikingRoomMaterial("VikingRoom_Material", "DefaultShader", vikingRoomTexture);
	Material defaultMaterial("DefaultMaterial", "DefaultShader", defaultTexture);

	m_renderer->CreateRenderer(vikingRoomMaterial);
	m_renderer->CreateTexture(*vikingRoomTexture.get());
	m_renderer->CreateTexture(*defaultTexture.get());

	m_renderer->CreateVertexBuffer(planeMesh.get());
	m_renderer->CreateIndexBuffer(planeMesh.get());

	m_renderer->CreateVertexBuffer(vikingRoomMesh.get());
	m_renderer->CreateIndexBuffer(vikingRoomMesh.get());

	uint32_t imageIndex = 0;

	//Create Scene
	std::shared_ptr<core::Scene> scene = std::make_shared<core::Scene>("Start Scene");
	
	// Camera:
	Entity camera_entity = core::ECSManager::CreateEntity("Camera", scene);
	int w, h;
	m_window->GetExtent(w, h);
	auto transform = camera_entity.GetComponent<core::Transform>();
	Camera& camera = camera_entity.AddComponet<Camera>(0.03f, 1000.f, 60.f, (float)w, (float)h);
	
	//HACKY ENTITIES:
	Entity vikingRoomEntity = core::ECSManager::CreateEntity("Viking Room", scene);
	core::Transform vikingroomTransform = vikingRoomEntity.GetComponent<core::Transform>();
	vikingroomTransform.matrix = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	Entity floorPlane = core::ECSManager::CreateEntity("Floor Plane", scene);
	core::Transform floorPlaneTransform = floorPlane.GetComponent<core::Transform>();
	floorPlaneTransform.matrix = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));


	//UBO:
	UniformBufferObject ubo = {};
	
	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();

		ubo.view = camera.GetView();
		ubo.proj = camera.GetProjection();

		if (gns::Input::GetKeyDown(SDLK_1))
			LOG_INFO("Key: 'W' was pressed!");

		if (gns::Input::GetKey(SDLK_1))
			LOG_INFO("Key: 'W' is held!");

		if (gns::Input::GetKeyUp(SDLK_1))
			LOG_INFO("Key: 'W' was released!");

		if (!m_renderer->BeginFrame(imageIndex)) continue;

		m_renderer->BeginDraw(imageIndex);
		m_renderer->UpdateUniformBuffer(imageIndex, ubo);
		
		ubo.model = vikingroomTransform.matrix;
		m_renderer->DrawMesh(vikingRoomMesh.get(), imageIndex, vikingRoomMaterial);

		ubo.model = floorPlaneTransform.matrix;
		m_renderer->DrawMesh(planeMesh.get(), imageIndex, defaultMaterial);
		
		m_renderer->EndDraw(imageIndex);
		m_renderer->EndFrame(imageIndex);
		Time::EndFrameTime();
	}
	m_renderer->DeleteMesh(planeMesh.get());
}

void gns::Application::CloseApplication()
{
	m_close = true;
}

void gns::Application::HandleEvents()
{
	m_close = m_window->PollEvents();
}

