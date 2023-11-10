#include "Application.h"
#include "Window/Window.h"
#include "Core/Time.h"
#include "AssetDatabase/Guid.h"
#include "Core/Scene.h"
#include "Log.h"

#include "Rendering/Renderer.h";

gns::Application::Application()
{
	m_window = new Window();
	m_close = false;
	m_renderer = new rendering::Renderer(m_window);
	/*Testing Randoms*/
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
}

gns::Application::~Application()
{
	delete(m_renderer);
	delete(m_window);
}

void gns::Application::Run()
{
	std::shared_ptr<Scene> scene = std::make_shared<Scene>("Default Scene");
	while (!m_close)
	{
		Time::StartFrameTime();
		HandleEvents();
		UpdateSystems();
		Render(scene);
		Time::EndFrameTime();
	}

}

void gns::Application::CloseApplication()
{
	m_close = true;
}

void gns::Application::Render(std::shared_ptr<Scene> scene)
{
	
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

