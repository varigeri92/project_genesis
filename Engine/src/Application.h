#pragma once
#include "gnsAPI.h"
#include "Core/Scene.h"
#include "Core/Components.h"
#include <functional>


#include "Rendering/RenderSystem.h"

namespace gns::rendering
{
	class Renderer;
}
namespace gns
{
	class GUI;
	class Window;
	class Application
	{
	public:
		Application(std::string assetsPath);
		GNS_API ~Application();
	private:
		//
		GUI* gui;
		Window* m_window;
		bool m_close;
		void CloseApplication();
		void Render(Scene* scene);
		void UpdateSystems();
		void UpdateLate();
	public:
		GNS_API void Start(std::function<void()> OnStart);
		GNS_API void Run(std::function<void()> OnUpdate);
		void HandleEvents();
	};
}