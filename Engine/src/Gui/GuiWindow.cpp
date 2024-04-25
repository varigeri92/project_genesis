#include "gnspch.h"
#include "GuiWindow.h"

void gns::gui::GuiWindow::DrawWindow()
{
	if (!m_isActive) return;
	OnBeforeWindowDraw();
	bool begin = ImGui::Begin(name.c_str(), &m_isActive, m_flags);
	if(begin)
	{
		OnGUI();
	}
	ImGui::End();
	OnAfterWindowDraw();
}

void gns::gui::GuiWindow::OnBeforeWindowDraw(){}

void gns::gui::GuiWindow::OnAfterWindowDraw()
{
}


gns::gui::GuiWindow::GuiWindow(const std::string name) : name(name), m_flags(ImGuiWindowFlags_None), m_instanceId(core::Guid::GetNewGuid())
{
}

void gns::gui::GuiWindow::SetActive(bool value)
{
	m_isActive = value;
}
