#pragma once

namespace gns::rendering
{
	class Device;
}
namespace gns
{
	class Window;
	
	class GUI
	{
		rendering::Device* m_device;
		Window* m_window;
		void InitializeGUI();
	public:
		GUI(rendering::Device* device, Window* window);

		void BeginGUI();
		void EndGUI();
		void DrawGUI();
		void DisposeGUI();
	};
}

