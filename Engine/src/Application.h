#pragma once
#include "Core/Scene.h"
#include "Core/Components.h"
#include "Systems/CameraSystem.h"

namespace gns::rendering
{
	class Renderer;
}
namespace gns
{
	class Window;

	class Application
	{
	public:
		Application();
		~Application();
	private:
		//
		Window* m_window;
		bool m_close;
		CameraSystem* camerasystem = nullptr;
		rendering::Renderer* m_renderer;
		void CloseApplication();
		void Render(std::shared_ptr<Scene> scene);
		void UpdateSystems();
		void UpdateLate();
	public:
		void Run();
		void HandleEvents();
	};
}