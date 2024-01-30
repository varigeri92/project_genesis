#include <GUI/GuiWindow.h>
#include <ImGui/imgui.h>

#include "GUI.h"

gns::gui::GuiWindow::GuiWindow(std::string name, GUI* gui) : Name(name)
{
	gui->RegisterWindow(this);
}

void gns::gui::GuiWindow::DrawWindow()
{
	ImGui::Begin(Name.c_str());
	OnGUI();
	ImGui::End();
}

