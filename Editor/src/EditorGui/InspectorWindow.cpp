﻿#include "InspectorWindow.h"

#include "../../../Engine/src/Gui/ImGui/IconsMaterialDesign.h"

ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit;

float dummyfloat[3] = {0,0,0};
ImVec2 ChildSize = { 0,0 };
ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;

bool show_component = true;

InspectorWindow::InspectorWindow() : GuiWindow("Inspector")
{
	
}

void InspectorWindow::OnBeforeWindowDraw()
{
	GuiWindow::OnBeforeWindowDraw();
}

void InspectorWindow::OnGUI()
{
	ChildSize.y = 0;

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