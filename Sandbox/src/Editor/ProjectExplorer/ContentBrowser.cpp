#include "ContentBrowser.h"
#include <filesystem>
#include <iostream>

#include "Log.h"
#include "ImGui/imgui.h"
#include "path.h"

namespace fs = std::filesystem;
float f =  0.5f;
std::string selected;
std::string currentPath;
std::string assetsPath;
ImGuiTreeNodeFlags nodeflags;

gns::gui::ContentBrowser::Directory gns::gui::ContentBrowser::SearchDirectory(gns::gui::ContentBrowser::Directory& directory)
{
    for (const auto& entry : fs::directory_iterator(directory.path))
    {
        if (is_directory(entry.path()))
        {
            directory.directories.emplace_back(entry.path().string(), entry.path().filename().string());
            SearchDirectory(directory.directories[directory.directories.size() - 1]);
        }
        else
        {
            directory.files.emplace_back(
                entry.path().string(), 
                entry.path().filename().string(), 
                entry.path().extension().string());
        }
    }
    return directory;
}

gns::gui::ContentBrowser::Directory gns::gui::ContentBrowser::DrawDirectory(Directory& path)
{
    return Directory();
}

gns::gui::ContentBrowser::Directory gns::gui::ContentBrowser::DrawLeftPane(Directory& path)
{
    return Directory();
}

gns::gui::ContentBrowser::ContentBrowser() : GuiWindow("Content Browser")
{
    m_flags = ImGuiWindowFlags_MenuBar;
    currentPath = ASSETS_DIR;
    selected = currentPath;
    assetsPath = currentPath;
    m_workingDirectory = {currentPath,selected };
    //SearchDirectory(m_workingDirectory);

    nodeflags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
}

void DrawdirectoryTree(fs::path path)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        ImGuiTreeNodeFlags flags = nodeflags;
        //entry.path().filename().string().c_str()
        if (is_directory(entry.path()))
        {
            if (!entry.path().empty())
            {
                bool containsDirectory = false;
                for (const auto& sub_entry : fs::directory_iterator(entry.path()))
                {
                    if (is_directory(sub_entry.path()))
                    {
                        containsDirectory = true;
                        break;
                    }
                }
                if (!containsDirectory)
                {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }

            }

            bool node_open = ImGui::TreeNodeEx(entry.path().string().c_str(), flags, entry.path().filename().string().c_str());
            if (ImGui::IsItemClicked())
            {
                selected = entry.path().string();
                LOG_INFO(selected);
            }

            if(node_open){
                DrawdirectoryTree(entry.path());
				ImGui::TreePop();
            }

        }
        /*
        if (ImGui::Selectable(entry.path().filename().string().c_str()))
        {
            selected = entry.path().string();
        }
        */
    }
}

void DrawDirectoryContent(fs::path path)
{
    ImVec2 button_sz(60 + (100 * f), 60 + (100 * f));
    ImGuiStyle& style = ImGui::GetStyle();
    int buttons_count = 20;
    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    int n = 0;
    for (const auto& entry : fs::directory_iterator(path))
    {
        ImGui::PushID(entry.path().string().c_str());
        if (is_directory(entry.path()))
        {
            ImGui::BeginGroup();
            if(ImGui::Button(entry.path().filename().string().c_str(), button_sz))
            {
                selected = entry.path().string();
				LOG_INFO(selected);
            }
            ImGui::TextWrapped(entry.path().filename().string().c_str());
            ImGui::EndGroup();
            
        }
        else
        {
            ImGui::BeginGroup();
            ImGui::Button(entry.path().filename().string().c_str(), button_sz);
            ImGui::TextWrapped(entry.path().filename().string().c_str());
            ImGui::EndGroup();
            
        }
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x;
        if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
            ImGui::SameLine();

        ImGui::PopID();
        //entry.path().filename().string().c_str()
        n++;
    }
}

void gns::gui::ContentBrowser::OnGUI()
{
    std::string path = ASSETS_DIR;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", "Ctrl+W")) { m_isOpen = false; }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

    DrawdirectoryTree(path);

    ImGui::EndChild();
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
    	ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

        if (ImGui::Button("  <<  "))
        {
            fs::path currentpath = selected;
        	selected = currentpath.parent_path().string();
            if (selected.length() < assetsPath.length())
                selected = assetsPath;

        }
        ImGui::SameLine();
        ImGui::SliderFloat("Icon Size", &f, 0, 1);
        ImGui::SeparatorText(selected.c_str());
        DrawDirectoryContent(selected);


        ImGui::EndChild();
    	ImGui::EndGroup();
    }
}

