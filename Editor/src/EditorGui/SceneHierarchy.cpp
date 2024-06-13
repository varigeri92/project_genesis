#include "SceneHierarchy.h"
#include "DockspaceWindow.h"
#include "Engine.h"
#include "../SelectionManager.h"

namespace gns::editor
{
	SceneHierarchy::SceneHierarchy() : GuiWindow("SceneHierarchy")
	{
		gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
		dockSpaceWindow->PushWindowMenu("SceneHierarchy", "", &m_isActive);
	}

	void SceneHierarchy::OnGUI()
	{
        ImGui::BeginChild("h");

		ImGui::Text("SceneHierarchy");
		auto entityView = SystemsAPI::GetRegistry().view<EntityComponent>();
        ImGui::TreePush("SceneHierarchy");
		for (auto [entt, entityComponent] : entityView.each())
		{
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth;
            if(SelectionManager::currentSelectedEntity == entt)
            {
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }
            /*
			ImGui::Text(entityComponent.name.c_str());
            ImGuiTreeNodeFlags node_flags = base_flags;
            const bool is_selected = (selection_mask & (1 << i)) != 0;
            if (is_selected)
                node_flags |= ImGuiTreeNodeFlags_Selected;
             
			if (i < 3)
            {
                // Items 0..2 are Tree Node
                bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    node_clicked = i;
                if (test_drag_and_drop && ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                    ImGui::Text("This is a drag and drop source");
                    ImGui::EndDragDropSource();
                }
                if (node_open)
                {
                    ImGui::BulletText("Blah blah\nBlah Blah");
                    ImGui::TreePop();
                }
            }
            else
            {
                // Items 3..5 are Tree Leaves
                // The only reason we use TreeNode at all is to allow selection of the leaf. Otherwise we can
                // use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
            }
            */

            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
            ImGui::TreeNodeEx((void*)&entt, node_flags, entityComponent.name.c_str());
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                SelectionManager::SetSelectedEntity(entt);
                LOG_INFO("Selected: " << entityComponent.name);
            }ImGui::SameLine();
            if(ImGui::Button("D"))
            {
                SelectionManager::SetSelectedEntity(entt::null);
                SystemsAPI::GetRegistry().destroy(entt);
            }
            
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                ImGui::Text("This is a drag and drop source");
                ImGui::EndDragDropSource();
            }
		}
        ImGui::TreePop();
        ImGui::EndChild();
        if (ImGui::BeginPopupContextItem("hierarchy_ctx_menu")) // <-- use last item id as popup id
        {
            if (ImGui::BeginMenu("< Create >"))
            {
                if (ImGui::MenuItem("Entity"))
                {
                    core::SceneManager::ActiveScene->CreateEntity("new entity");
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
	}
}
