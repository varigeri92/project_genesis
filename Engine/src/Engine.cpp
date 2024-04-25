#include <gnspch.h>
#include "../API/Engine.h"

#include "AssetDatabase/AssetLoader.h"
#include "SystemsApi/ComponentLibrary.h"
#include "Renderer/Rendering.h"
#include "../Utils/CameraSystem.h"
#include "Gui/GuiSystem.h"
#include "Window/Screen.h"
#include "Window/Screen.h"

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

gns::Engine::Engine() :close(false), m_window(new Window(1920, 1080))
{
	START_PROFILER("Engine")
	PROFILE_FUNC
	Screen::InitDefaultScreen(1920, 1080, Screen::ScreenMode::sm_none);
}

gns::Engine::~Engine()
{
	PROFILE_FUNC
	delete m_window;
#ifdef TRACE_ALLOCATION
	LOG_TRACE("Current memory usage = " << trace.GetAllocations());
#endif
	END_PROFILER
}

void gns::Engine::Init(std::function<void()> startupCallback)
{

	RenderSystem* renderSystem = SystemsAPI::RegisterSystem<RenderSystem>(m_window);
	m_guiSystemInstance = new gns::gui::GuiSystem(renderSystem->GetDevice(), m_window);
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
	// Entity tests:
	Entity cameraEntity = Entity::CreateEntity("Camera");
	Transform& cameraTransform = cameraEntity.GetComponent<Transform>();
	Camera& cam = cameraEntity.AddComponet<Camera>(0.01, 1000, 60, 1920, 1080, cameraTransform);
	cam._near = 0.01f;
	cam._far = 1000.f;
	LOG_INFO(cam._near);
	LOG_INFO(cam._far);

	cameraTransform.position = { 0.f,-1.f,-5.f };
	
	CameraSystem* cameraSystem = SystemsAPI::RegisterSystem<CameraSystem>();
	cameraSystem->UpdateProjection(1920, 1080);
	const std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("blinphong.vert.spv", "blinphong.frag.spv");
	renderSystem->GetRenderer()->CreatePipeline(defaultShader);

	const std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>(R"(Textures\uv_color_Grid.png)");

	const std::shared_ptr<Material> defaultMaterial = std::make_shared<Material>(defaultShader, "default Material");
	defaultMaterial->SetTexture(defaultTexture, 0);

	const auto meshInstance = AssetLoader::LoadMeshFile(R"(Meshes\Suzan.obj)");
	LOG_INFO(meshInstance.size());
	for (const std::shared_ptr<Mesh>& mesh : meshInstance)
	{
		LOG_INFO(mesh->name);
		renderSystem->GetRenderer()->UploadMesh(mesh.get());
		Entity reiEntity = Entity::CreateEntity(mesh->name);
		RendererComponent& rc = reiEntity.AddComponet<RendererComponent>(mesh, defaultMaterial);
		reiEntity.GetComponent<Transform>().position = { 0,0,0 };
	}
}

void gns::Engine::Run()
{
	while(!close)
	{
		PROFILE_FUNC
		Time::StartFrameTime();
		close = m_window->PollEvents();
		m_guiSystemInstance->BeginGUI();
		m_guiSystemInstance->UpdateGui();
		SystemsAPI::UpdateSystems(Time::GetDelta());
		Time::EndFrameTime();
	}
}

void gns::Engine::ShutDown()
{
	PROFILE_FUNC

	m_guiSystemInstance->DisposeGUI();
	RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	renderSystem -> CleanupRenderer();
	SystemsAPI::ClearSystems();

	TestEvent.Dispatch("This is a Dispatch message test!");
	delete CallThis_EventFunction_InClass;
}
