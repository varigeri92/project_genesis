#pragma once
#include <GUI/GuiWindow.h>

using namespace gns::gui;
namespace gns
{
	
	class TestWindow : public GuiWindow
	{
	public:
		TestWindow(std::string name) : GuiWindow(name) {};

	protected:
		void OnGUI() override;
	};
}
