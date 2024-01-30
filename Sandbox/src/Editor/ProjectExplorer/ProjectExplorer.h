#pragma once
#include <GUI/GuiWindow.h>

using namespace gns::gui;
class ProjectExplorer : GuiWindow
{
public:
	ProjectExplorer(std::string name, gns::GUI* gui) : GuiWindow(name, gui) {};

protected:
	GEN_API void OnGUI() override;
};

