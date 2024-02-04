#pragma once
#include <GUI/GuiWindow.h>

using namespace gns::gui;
class ProjectExplorer : GuiWindow
{
public:
	ProjectExplorer(std::string name) : GuiWindow(name) {};

protected:
	GEN_API void OnGUI() override;
};

