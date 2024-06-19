#include "DockspaceWindow.h"
#include "Engine.h"
#include "EditorStyles.h"


gns::editor::DockspaceWindow::DockspaceWindow() : GuiWindow("Dockspace")
{
    m_flags = ImGuiWindowFlags_MenuBar| ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    styles::DeepDark();
}

void gns::editor::DockspaceWindow::OnGUI()
{
    m_windowID = ImGui::GetID(name.c_str());
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(m_windowID, ImVec2(0.0f, 0.0f), 0);
    bool dummy = false;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Save Scene ...", "Ctrl + S", &dummy))
            {
                std::string path = NativeFileBrowser::OpenBrowser(".gnsscene", NativeFileBrowser::FileBrowserMode::SaveFile);
				core::SceneManager::SaveSceneToFile(path);
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Open Scene ...", "Ctrl + O", &dummy))
            {
                std::string path = NativeFileBrowser::OpenBrowser(".gnsscene", NativeFileBrowser::FileBrowserMode::SaveFile);
                core::SceneManager::loadSceneFromFile(path);
            }
            ImGui::MenuItem("dummy ...", "Ctrl + O", &dummy);
        	ImGui::Separator();
        	ImGui::EndMenu();
        }
        BuildWindowMenu();
        ImGui::EndMenuBar();
    }

}

void gns::editor::DockspaceWindow::OnBeforeWindowDraw()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	GuiWindow::OnBeforeWindowDraw();
}

void gns::editor::DockspaceWindow::BuildWindowMenu() const

{
    if (ImGui::BeginMenu("Window"))
    {
        for(const auto item : m_windowItems)
        {
        	if(ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str()))
        	{
                const bool open = *item.windowOpen_ptr;
				*item.windowOpen_ptr = !open;
        	}
        }
        ImGui::Separator();
        if (ImGui::MenuItem("ImGui Demo Window", 0))
        {
            gui::GuiSystem::ToggleDemoWindow();
        }
        ImGui::Separator();
        ImGui::EndMenu();
    }
}

void gns::editor::DockspaceWindow::PushWindowMenu(std::string label, std::string shortCut, bool* windowOpen)
{
    m_windowItems.emplace_back(label, shortCut, windowOpen);
}
