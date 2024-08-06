#include <gnspch.h>
#include "../API/Engine.h"

#include "AssetDatabase/AssetLoader.h"
#include "SystemsApi/ComponentLibrary.h"
#include "Renderer/Rendering.h"
#include "../Utils/CameraSystem.h"
#include "Gui/GuiSystem.h"
#include "Level/SceneManager.h"
#include "Window/Screen.h"
#include "Window/Screen.h"
#include "Utils/Serialization/Serializer.h"

#ifdef TRACE_ALLOCATION
struct AllocationTrace
{
	inline static size_t Allocated = 0;
	static size_t GetAllocations()
	{
		return (Allocated);
	}
};
AllocationTrace trace;
#endif

void* operator new(size_t size)
{
#ifdef TRACE_ALLOCATION
	trace.Allocated += size;
#endif
	void* p = malloc(size);
	return p;
}

void operator delete(void* p, size_t size)
{
#ifdef TRACE_ALLOCATION
	trace.Allocated -= size;
#endif
	free(p);
}
//TESTING:
gns::EventFunction<void, std::string> DoNotCallThis_EventFunction([](const std::string& message)
	{
		LOG_INFO("Do not Call this: " << message);
	});
// End Testing!

using namespace  gns::rendering;

gns::Engine::Engine() :close(false)
{
	START_PROFILER("Engine")
	PROFILE_FUNC

	Window::getInstance()->InitWindow(1920, 1080);
	Screen::InitDefaultScreen(1920, 1080, Screen::ScreenMode::sm_none);
	Serializer::RegisterSerializableComponents([](){
		gns::RegisterComponent<gns::EntityComponent>();
		gns::RegisterComponent<gns::Transform>();
		gns::RegisterComponent<gns::Camera>();
		gns::RegisterComponent<gns::RendererComponent>();

		gns::RegisterComponent<gns::Light>();
		gns::RegisterComponent<gns::Ambient>();
		gns::RegisterComponent<gns::LightDirection>();
		gns::RegisterComponent<gns::SpotLight>();
		gns::RegisterComponent<gns::PointLight>();
		gns::RegisterComponent<gns::ShadowCaster>();

	});
}

gns::Engine::~Engine()
{
	PROFILE_FUNC
#ifdef TRACE_ALLOCATION
	LOG_TRACE("Current memory usage = " << trace.GetAllocations());
#endif
	END_PROFILER
}

void gns::Engine::Init(std::function<void()> startupCallback)
{
	AssetDatabase::LoadAssetDatabase();
	Scene* scene =  core::SceneManager::CreateScene("DefaultScene");
	RenderSystem* renderSystem = SystemsAPI::RegisterSystem<RenderSystem>();
	m_guiSystemInstance = new gns::gui::GuiSystem(renderSystem->GetDevice());
	startupCallback();


	// Event system subsribe tests:
	CallThis_EventFunction_InClass = new gns::EventFunction<void, std::string>([](const std::string& message)
	{
		LOG_INFO("Test Function Located inside the class: " << message);
	});

	CallThis_EventFunction_InClass->Call("CAll Message!");
	TestEvent.Subscribe<void, std::string>(&DoNotCallThis_EventFunction);
	TestEvent.Subscribe<void, std::string>(CallThis_EventFunction_InClass);
	TestEvent.RemoveListener<void, std::string>(&DoNotCallThis_EventFunction);

}

void gns::Engine::Run()
{
	while(!close)
	{
		PROFILE_FUNC
		Time::StartFrameTime();
		close = Window::getInstance()->PollEvents();
		if(!Window::getInstance()->isMinimized)
		{
			m_guiSystemInstance->BeginGUI();
			m_guiSystemInstance->UpdateGui();
			SystemsAPI::UpdateSystems(Time::GetDelta());
		}
		Time::EndFrameTime();
	}
}

void gns::Engine::ShutDown()
{
	PROFILE_FUNC

	m_guiSystemInstance->DisposeGUI();
	Object::DisposeAll();
	SystemsAPI::ClearSystems();

	TestEvent.Dispatch("This is a Dispatch message test!");
	delete CallThis_EventFunction_InClass;
}
