#pragma once
#include <vector>

#include "GUI/GuiWindow.h"

namespace gns::rendering
{
	class Device;
}
namespace gns
{
	class Window;
	class GUI
	{
	friend class gns::gui::GuiWindow;
	private:
		rendering::Device* m_device;
		Window* m_window;
		void InitializeGUI();
		std::vector<gns::gui::GuiWindow*> guiWindows;
		void RegisterWindow(gns::gui::GuiWindow* gui_window);
	public:
		GUI(rendering::Device* device, Window* window);

		void BeginGUI();
		void EndGUI();
		void DrawGUI();
		void DisposeGUI();
	};
}

