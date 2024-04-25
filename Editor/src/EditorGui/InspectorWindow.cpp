#include "InspectorWindow.h"
#include "../../../Engine/src/Gui/ImGui/IconsMaterialDesign.h"
#include "../SelectionManager.h"
#include "DockspaceWindow.h"

ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit;

float dummyfloat[3] = {0,0,0};
ImVec2 ChildSize = { 0,0 };
ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

bool show_component = true;
entt::entity nullentity = entt::null;
InspectorWindow::InspectorWindow() : GuiWindow("Inspector"), inspectedEntity(nullentity)
{
	gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
	dockSpaceWindow->PushWindowMenu("Inspector Window", "", &m_isActive);
	onEntitySelected = new gns::EventFunction<void, entt::entity>([&](entt::entity selectedEntity)
	{
			LOG_INFO(static_cast<uint64_t>(selectedEntity));
			inspectedEntity = gns::Entity(selectedEntity);
	});
	SelectionManager::onSelectionChanged.Subscribe<void, entt::entity>(onEntitySelected);
}

void InspectorWindow::OnBeforeWindowDraw()
{
	GuiWindow::OnBeforeWindowDraw();
}

void InspectorWindow::OnGUI()
{
	ChildSize.y = 0;
	if(inspectedEntity.IsValid())
		ImGui::Text(inspectedEntity.GetComponent<gns::EntityComponent>().name.c_str());

	DrawComponent("Transform");
	DrawComponent("Transform_2");

}

void InspectorWindow::OnAfterWindowDraw()
{
	GuiWindow::OnAfterWindowDraw();
}

inline void InspectorWindow::DrawComponent(std::string name)
{
	ImGui::BeginChild(name.c_str(), ChildSize, child_flags);

	ImGui::PushFont(gns::gui::GuiSystem::boldFont);

	ImGui::Text(name.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() - 80);
	if(ImGui::Button(ICON_MD_ARROW_DROP_DOWN))
		show_component = !show_component;

	ImGui::SameLine(ImGui::GetWindowWidth() - 40);
	ImGui::Button(ICON_MD_DELETE_FOREVER);

	ImGui::Separator();
	ImGui::PopFont();
	if(show_component)
	{
		ImGui::BeginTable(name.c_str(), 2, table_flags);

		DrawField(120, &dummyfloat, "Position");
		DrawField(121, &dummyfloat, "Rotation");
		DrawField(122, &dummyfloat, "Scale");

		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void InspectorWindow::DrawField(size_t typeId, void* valuePtr, std::string name)
{
	std::string fieldName = "##" + name;
	ImGui::TableNextRow();
	ImGui::TableNextColumn(); ImGui::Text(name.c_str());
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::DragFloat3(fieldName.c_str(), static_cast<float*>(valuePtr));
	ImGui::PopItemWidth();
}
