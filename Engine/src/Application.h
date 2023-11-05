#pragma once


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
		Window* m_window;
		bool m_close;
		rendering::Renderer* m_renderer;
		void CloseApplication();
	public:
		void Run();
		void HandleEvents();
	};
}

