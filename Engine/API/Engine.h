#pragma once
#include <functional>

#include "api.h"
#include "Log.h"
#include "../src/Utils/Event.h"
#include "../src/Utils/Time.h"
#include "../src/Utils/CameraSystem.h"

#include "../src/Window/InputBackend.h"
#include "../src/SystemsApi/SystemsAPI.h"
#include "../src/SystemsApi/Entity.h"
#include "../src/SystemsApi/ComponentLibrary.h"
#include "../src/Window/Window.h"
#include "../src/Window/Screen.h"
#include "../src/Gui/GuiSystem.h"
#include "../src/Gui/GuiWindow.h"
#include "../src/Gui/ImGui/imgui.h"
#include "../src/Renderer/Rendering.h"
#include "../src/Utils/Serialization/Serializer.h"
#include "../src/Object/IDisposable.h"
#include "../src/Object/Object.h"

namespace gns
{
	class Window;

	class Engine
	{
	public:
		gns::Event<void, std::string> TestEvent;
		bool close;
	public:
		GNS_API Engine();
		GNS_API ~Engine();

		Engine(Engine& other) = delete;
		Engine operator=(Engine& other) = delete;

		GNS_API void Init(std::function<void()> startupCallback);
		GNS_API void Run();
		GNS_API void ShutDown();
		GNS_API Window* GetWindow() const { return m_window; };
	private:
		Window* m_window;
		gns::gui::GuiSystem* m_guiSystemInstance;
		gns::EventFunction<void, std::string>* CallThis_EventFunction_InClass;
	};
}