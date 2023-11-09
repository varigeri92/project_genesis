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

UniformBufferObject ubo = {};
void gns::Application::Run()
{

	auto vikingRoomTexture = std::make_shared<Texture>(R"(D:\project_genesis\Engine\Assets\Textures\viking_room.png)");
	auto defaultTexture = std::make_shared<Texture>(R"(D:\project_genesis\Engine\Assets\Textures\PrototypeGridTexture.jpg)");
	auto planeMesh = gns::AssetLoader::LoadMesh(R"(D:\project_genesis\Engine\Assets\Meshes\plane.obj)");
	auto vikingRoomMesh = gns::AssetLoader::LoadMesh(R"(D:\project_genesis\Engine\Assets\Meshes\viking_room.obj)");
	auto vikingRoomMaterial = std::make_shared <Material>("VikingRoom_Material", "DefaultShader", vikingRoomTexture);
	auto defaultMaterial = std::make_shared <Material>("DefaultMaterial", "DefaultShader", defaultTexture);


	std::shared_ptr<gns::core::Scene> scene = std::make_shared<gns::core::Scene>("Start Scene");

	// Camera:
	auto camera_entity = gns::core::ECSManager::CreateEntity("Camera", scene);

	auto camera_transform = camera_entity.GetComponent<gns::core::Transform>();
	camera_transform.position = glm::vec3(0, -3, 1);
	//HACKY ENTITIES:
	auto vikingRoomEntity = gns::core::ECSManager::CreateEntity("Viking Room", scene);
	gns::core::Transform& vikingroomTransform = vikingRoomEntity.GetComponent<gns::core::Transform>();
	vikingroomTransform.matrix = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(230.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	vikingRoomEntity.AddComponet<gns::core::MeshComponent>(vikingRoomMesh);
	vikingRoomEntity.AddComponet<gns::core::MaterialComponent>(vikingRoomMaterial);

	auto floorPlane = gns::core::ECSManager::CreateEntity("Floor Plane", scene);
	gns::core::Transform& floorPlaneTransform = floorPlane.GetComponent<gns::core::Transform>();
	floorPlaneTransform.matrix = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	floorPlane.AddComponet<gns::core::MeshComponent>(planeMesh);
	floorPlane.AddComponet<gns::core::MaterialComponent>(defaultMaterial);


	m_renderer->CreateRenderer(*(vikingRoomMaterial.get()));
	m_renderer->CreateTexture(*vikingRoomTexture.get());
	m_renderer->CreateTexture(*defaultTexture.get());

	m_renderer->CreateVertexBuffer(planeMesh.get());
	m_renderer->CreateIndexBuffer(planeMesh.get());

	m_renderer->CreateVertexBuffer(vikingRoomMesh.get());
	m_renderer->CreateIndexBuffer(vikingRoomMesh.get());

	floorPlane.GetComponent<gns::core::EntityComponent>().isEnabled = true;
	vikingRoomEntity.GetComponent<gns::core::EntityComponent>().isEnabled = true;

	//Create Scene

	//UBO:
	int w, h;
	m_window->GetExtent(w, h);
	//Test Stuff:



	auto camera = camera_entity.AddComponet<Camera>(0.03f, 1000.f, 60.f, (float)w, (float)h);
	camerasystem = new gns::core::CameraSystem(camera_transform, camera);

	ubo.view = camera.GetView();
	ubo.proj = camera.GetProjection();
	
	float rotation = 1;
	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();
		rotation += Time::GetDelta() * 0.25f;
		vikingroomTransform.matrix = glm::rotate(glm::mat4(1.0f), rotation * glm::radians(230.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//UpdateSystems(vikingroomTransform);
		camerasystem->UpdateSystem();
		int w, h;
		m_window->GetExtent(w, h);
		camerasystem->UpdateProjection(w, h);
		ubo.view = camera.GetView();
		ubo.proj = camera.GetProjection();
		UpdateLate();

		Render(scene);
		
		Time::EndFrameTime();
	}
	m_renderer->DeleteMesh(planeMesh.get());
	m_renderer->DeleteMesh(vikingRoomMesh.get());

	m_renderer->DeleteTexture(vikingRoomTexture.get());
	m_renderer->DeleteTexture(defaultTexture.get());
}

void gns::Application::CloseApplication()
{
	m_close = true;
}

void gns::Application::Render(std::shared_ptr<gns::core::Scene> scene)
{
	uint32_t imageIndex = 0;
	if (!m_renderer->BeginFrame(imageIndex)) {
		return;
	}
	//LOG_INFO("START RENDERING!");
	auto view = scene->registry.view<
		gns::core::Transform, 
		gns::core::MeshComponent, 
		gns::core::MaterialComponent, 
		gns::core::EntityComponent>();
	

	view.each([this, imageIndex](auto &transform, auto &mesh, auto& material, auto& entityComponent) {
		ubo.model = transform.matrix;
		m_renderer->UpdateUniformBuffer(imageIndex, ubo);

		m_renderer->BeginDraw(material.material.get());
		m_renderer->Submit(mesh.mesh.get());
		m_renderer->EndDraw(imageIndex);
	});

	m_renderer->EndFrame(imageIndex);
}

void gns::Application::UpdateSystems(gns::core::Transform& _transform)
{

	float x_velocity = 0.f;
	float y_velocity = 0.f;
	float z_velocity = 0.f;

	//fwd:
	if (Input::GetKey(SDLK_w)) {
		z_velocity = 1.f;
	}
	//back:
	if (Input::GetKey(SDLK_s)) {
		z_velocity = -1.f;
	}

	//left:
	if (Input::GetKey(SDLK_a)) {
		x_velocity = 1.f;
	}
	//right:
	if (Input::GetKey(SDLK_d)) {
		x_velocity = -1.f;
	}

	//up:
	if (Input::GetKey(SDLK_q)) {
		y_velocity = 1.f;
	}
	//down:
	if (Input::GetKey(SDLK_e)) {
		y_velocity = -1.f;
	}

	_transform.position += glm::vec3(x_velocity, y_velocity, z_velocity) * (Time::GetDelta() * 1);
	_transform.UpdateMatrix();
	LOG_INFO("Current position: " << _transform.position.x << " / " << _transform.position.y << " / " << _transform.position.z);
}

void gns::Application::UpdateLate()
{
}

void gns::Application::HandleEvents()
{
	m_close = m_window->PollEvents();
}

