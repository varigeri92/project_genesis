#include <GUI/GuiWindow.h>
#include <ImGui/imgui.h>

#include "GUI.h"


gns::gui::GuiWindow::GuiWindow(std::string name) : Name(name)
{
	GUI::RegisterWindow(this);
}

void gns::gui::GuiWindow::DrawWindow()
{
	ImGui::Begin(Name.c_str(), &m_isOpen, m_flags);
	OnGUI();
	ImGui::End();
}

