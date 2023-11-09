#pragma once
#include "Core/Scene.h"
#include "Core/Components.h"
#include "Systems/CameraSystem.h"
namespace gns::rendering
{
	class Renderer;
	class Device;
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
		rendering::Renderer* m_renderer;
		gns::core::CameraSystem* camerasystem = nullptr;
		void CloseApplication();
		void Render(std::shared_ptr<gns::core::Scene> scene);
		void UpdateSystems(gns::core::Transform& transform);
		void UpdateLate();
	public:
		void Run();
		void HandleEvents();
	};
}