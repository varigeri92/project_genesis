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

    void DrawHierarchyRecursive(entt::entity e, EntityComponent& entitycomponent)
	{
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth;
        Entity entity = { e };
        std::vector<Entity> children = entity.GetChildren();
        if(children.size() == 0)
        {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        if (SelectionManager::currentSelectedEntity == e)
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (ImGui::TreeNodeEx(entitycomponent.name.c_str(), node_flags))
        {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                SelectionManager::SetSelectedEntity(e);
                LOG_INFO("Selected: " << entitycomponent.name);
            }
        }
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("Entity", NULL, 0);
            ImGui::Text(entitycomponent.name.c_str());
            ImGui::EndDragDropSource();
        }
		for (auto& child : children)
        {
            DrawHierarchyRecursive(child.entity, child.GetComponent<EntityComponent>());
        }
	}

	void SceneHierarchy::OnGUI()
	{
        ImGui::BeginChild("h");

		ImGui::Text("SceneHierarchy");
        ImGui::TreePush("SceneHierarchy");
		auto entityView = SystemsAPI::GetRegistry().view<EntityComponent>();
        ImGui::TreePop();
        ImGui::Separator();
        for (auto [entt, entityComponent] : entityView.each())
        {
            DrawHierarchyRecursive(entt, entityComponent);
        }
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
