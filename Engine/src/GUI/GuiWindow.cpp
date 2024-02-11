#include <GUI/GuiWindow.h>
#include <ImGui/imgui.h>

#include "GUI.h"


gns::gui::GuiWindow::GuiWindow(std::string name) : Name(name)
{

}

void gns::gui::GuiWindow::SetActive(bool value)
{
	m_isActive = value;
}

void gns::gui::GuiWindow::DrawWindow()
{
	if (!m_isActive) return;
	ImGui::Begin(Name.c_str(), &m_isActive, m_flags);
	OnGUI();
	ImGui::End();
}

