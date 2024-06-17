#include "SceneHierarchy.h"
#include "DockspaceWindow.h"
#include "Engine.h"
#include "../SelectionManager.h"
#include "../DragDropManager.h"

namespace gns::editor
{
    std::string treenodeID = "##";
	SceneHierarchy::SceneHierarchy() : GuiWindow("SceneHierarchy")
	{
		gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
		dockSpaceWindow->PushWindowMenu("SceneHierarchy", "", &m_isActive);
	}

    void DrawHierarchyRecursive(entt::entity e, EntityComponent& entitycomponent, bool DrawEntity = false)
	{
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth; //| ImGuiTreeNodeFlags_DefaultOpen;
        Entity entity = { e };
        std::vector<Entity> children = entity.GetChildren();
        bool is_leaf = false;
        if(children.size() == 0)
        {
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            is_leaf = true;
        }
        if (SelectionManager::currentSelectedEntity == e)
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (DrawEntity) {
            if(entitycomponent.name == "")
            {
                treenodeID = "##";
            }
            else
            {
                treenodeID = entitycomponent.name;
            }
            bool node_open = ImGui::TreeNodeEx(treenodeID.c_str(), node_flags);
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
                {
                    Payload* pl = DragDropManager::GetCurrentPayload();
                    if (pl->type == PayloadType::Entity) {
                        Entity drag_entity = ((entt::entity)pl->entity_id);
                        LOG_INFO("Dropped: "  << drag_entity.GetComponent<EntityComponent>().name << " onto " << entitycomponent.name);
                        drag_entity.SetParent(entity);
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                SelectionManager::SetSelectedEntity(e);
                LOG_INFO("Selected: " << entitycomponent.name);
            }
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("ENTITY", NULL, 0);
                ImGui::Text(entitycomponent.name.c_str());
                DragDropManager::SetCurrentPayload_Entity((uint32_t)e);
                ImGui::EndDragDropSource();
            }
            if (node_open && !is_leaf) {
		        for (auto& child : children)
                {
                    DrawHierarchyRecursive(child.entity, child.GetComponent<EntityComponent>(), true);
                }
                ImGui::TreePop();
            }
        }
        else {
            for (auto& child : children)
            {
                DrawHierarchyRecursive(child.entity, child.GetComponent<EntityComponent>(), true);
            }
        }
        
	}

	void SceneHierarchy::OnGUI()
	{
        ImGui::BeginChild("h");

		ImGui::Text("SceneHierarchy");
        Entity& root = core::SceneManager::ActiveScene->GetSceneRoot();
        EntityComponent ec = root.GetComponent<EntityComponent>();
        ImGui::Separator();
        DrawHierarchyRecursive(root.entity, ec);
        ImGui::EndChild();
        if (ImGui::BeginPopupContextItem("hierarchy_ctx_menu")) // <-- use last item id as popup id
        {
            if (ImGui::BeginMenu("< Create >"))
            {
                if (ImGui::MenuItem("Entity"))
                {
                    Entity entity = core::SceneManager::ActiveScene->CreateEntity("new entity");
                    SelectionManager::SetSelectedEntity(entity.entity);
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

	}
}
