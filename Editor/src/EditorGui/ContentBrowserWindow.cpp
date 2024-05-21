#include "ContentBrowserWindow.h"
#include "DockspaceWindow.h"
#include <string>
#include <iostream>
#include <filesystem>

#include "../DragDropManager.h"
#include "../../../Engine/src/AssetDatabase/AssetLoader.h"
#include "../../../Engine/src/Gui/ImGui/IconsMaterialDesign.h"

namespace fs = std::filesystem;
static float iconSize = 1;
static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit;
static float dummyfloat[3] = { 0,0,0 };
static ImVec2 ChildSize = { 0,0 };
static ImVec2 button_sz(120, 120);
static ImVec2 buttonParent_sz(130, 150);
static ImVec2 defaultButton_sz(105, 105);
static ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

static std::shared_ptr<gns::rendering::Texture> DirectoryIcon;
static std::shared_ptr<gns::rendering::Texture> FileIcon;


namespace gns::editor
{

	std::string base_name(std::string const& path)
	{
		return path.substr(path.find_last_of("/\\") + 1);
	}

	ContentBrowserWindow::ContentBrowserWindow() : GuiWindow("ContentBrowserWindow")
	{
		gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
		dockSpaceWindow->PushWindowMenu("Content Browser Window", "", &m_isActive);
		m_assetsPath = AssetLoader::GetAssetsPath();
		m_currentDirPath = m_assetsPath;
		m_parentDirPath = m_assetsPath;
		button_sz.x = defaultButton_sz.x * iconSize;
		button_sz.y = defaultButton_sz.y * iconSize;

		DirectoryIcon = std::make_shared<gns::rendering::Texture>(R"(Icons\open-folder.png)", true);
		DirectoryIcon->Apply();

		DirectoryIcon->m_descriptorSet = ImGui_ImplVulkan_AddTexture(DirectoryIcon->m_sampler, DirectoryIcon->m_imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		FileIcon = std::make_shared<gns::rendering::Texture>(R"(Icons\icon_file.png)", true);
		FileIcon->Apply();

		FileIcon->m_descriptorSet = ImGui_ImplVulkan_AddTexture(FileIcon->m_sampler, FileIcon->m_imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
 
	}

	ContentBrowserWindow::~ContentBrowserWindow()
	{
		DirectoryIcon->Destroy();
		FileIcon->Destroy();
	}

	void ContentBrowserWindow::OnGUI()
	{
		ImGui::BeginChild("ContentBrowserHeader", ChildSize, ImGuiChildFlags_AutoResizeY);
		if(ImGui::Button(ICON_MD_ARROW_BACK))
		{
			m_currentDirPath = m_parentDirPath;
		}
		ImGui::SameLine();
		ImGui::SeparatorText(base_name(m_currentDirPath).c_str());
		ImGui::EndChild();

		ImGui::BeginChild("Left-pane", ImVec2(250, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		{
			ImGui::TreePush("SceneHierarchy");

			//DrawDirectoryInHierarchy(m_currentDirPath);
			ImGui::TreePop();
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::BeginChild("Right-pane", ImVec2(0, 0));
		{
			ImGuiStyle& style = ImGui::GetStyle();
			int buttons_count = 20;
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

			int buttonsInRow = 0;
			for (const auto& entry : fs::directory_iterator(m_currentDirPath))
			{
				ImGui::PushID(entry.path().string().c_str());
				if (entry.is_directory())
					DrawDirectory(entry);
				else
					DrawFile(entry);
				
				//ImGui::EndDragDropSource();
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
				if (buttonsInRow + 1 < buttons_count && next_button_x2 < window_visible_x2)
					ImGui::SameLine();

				ImGui::PopID();
				buttonsInRow++;
			}
		}
		ImGui::EndChild();
		ImGui::EndGroup();
	}

	void ContentBrowserWindow::DrawDirectory(const std::filesystem::directory_entry& entry)
	{
		ImGui::BeginChild((entry.path().string() + "_button").c_str(), buttonParent_sz, child_flags);
		if (ImGui::ImageButton(base_name(entry.path().string()).c_str(), DirectoryIcon->m_descriptorSet,button_sz))
		{
			m_currentDirPath = entry.path().string();
		}
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("CONTENT_BROWSER_DIR", nullptr, 0);
			ImGui::Text(base_name(entry.path().string()).c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::Text(base_name(entry.path().string()).c_str());
		ImGui::EndChild();
	}

	void ContentBrowserWindow::DrawFile(const std::filesystem::directory_entry& entry)
	{

		ImGui::BeginChild((entry.path().string() + "_button").c_str(), buttonParent_sz, child_flags);
		if (ImGui::ImageButton(base_name(entry.path().string()).c_str(), FileIcon->m_descriptorSet, button_sz))
		{
			LOG_INFO("Opnening or inspecting file: '" << entry.path().string() <<"' feature not implemented...");
		}
		if (ImGui::BeginDragDropSource())
		{
			if(ImGui::SetDragDropPayload("CONTENT_BROWSER_FILE", nullptr, 0))
			{
				DragDropManager::SetCurrentPayload(nullptr, entry.path().string());
			}
			
			ImGui::Text(base_name(entry.path().string()).c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::Text(base_name(entry.path().string()).c_str());
		ImGui::EndChild();
	}

	void ContentBrowserWindow::DrawDirectoryInHierarchy(const std::string& path)
	{
		for (const auto& entry : fs::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				if(ImGui::TreeNodeEx(base_name(entry.path().string()).c_str()))
				{
					 //DrawDirectoryInHierarchy(entry.path().string());
				}
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
					ImGui::Text("This is a drag and drop source");
					ImGui::EndDragDropSource();
				}
			}
		}
	}
}
