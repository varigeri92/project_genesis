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
		static std::vector<gns::gui::GuiWindow*> guiWindows;
		static void RegisterWindow(gns::gui::GuiWindow* gui_window);
	public:
		GUI(rendering::Device* device, Window* window);
		static GEN_API gns::gui::GuiWindow* GetWindow_Internal(std::string name);


		void BeginGUI();
		void EndGUI();
		void DrawGUI();
		void DisposeGUI();
	};
}

