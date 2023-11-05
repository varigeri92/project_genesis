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

const std::vector<gns::Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};

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
	std::shared_ptr<Texture> vikingRoomTexture = std::make_shared<Texture>(R"(D:\GenesisEngine\Engine\Assets\Textures\viking_room.png)");
	std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>(R"(D:\GenesisEngine\Engine\Assets\Textures\PrototypeGridTexture.jpg)");
	std::shared_ptr<gns::Mesh> planeMesh = AssetLoader::LoadMesh(R"(D:\GenesisEngine\Engine\Assets\Meshes\plane.obj)"); //std::make_shared<Mesh>(vertices, indices);
	std::shared_ptr<gns::Mesh> vikingRoomMesh = AssetLoader::LoadMesh(R"(D:\GenesisEngine\Engine\Assets\Meshes\viking_room.obj)"); //std::make_shared<Mesh>(vertices, indices);
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


	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();

		if (gns::Input::GetKeyDown(SDLK_1))
			LOG_INFO("Key: 'W' was pressed!");

		if (gns::Input::GetKey(SDLK_1))
			LOG_INFO("Key: 'W' is held!");

		if (gns::Input::GetKeyUp(SDLK_1))
			LOG_INFO("Key: 'W' was released!");

		if (!m_renderer->BeginFrame(imageIndex)) continue;
		m_renderer->UpdateUniformBuffer(imageIndex);
		m_renderer->BeginDraw(imageIndex);
		m_renderer->DrawMesh(vikingRoomMesh.get(), imageIndex, vikingRoomMaterial);
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

