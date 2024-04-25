#include "ContentBrowserWindow.h"
#include "DockspaceWindow.h"

namespace gns::editor
{
	ContentBrowserWindow::ContentBrowserWindow() : GuiWindow("ContentBrowserWindow")
	{
		gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
		dockSpaceWindow->PushWindowMenu("Content Browser Window", "", &m_isActive);
	}

	void ContentBrowserWindow::OnGUI()
	{
		ImGui::Text("ContentBrowserWindow");
	}
}
