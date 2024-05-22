#include "InspectorWindow.h"
#include "../../../Engine/src/Gui/ImGui/IconsMaterialDesign.h"
#include "../SelectionManager.h"
#include "DockspaceWindow.h"


static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit;
static ImVec2 ChildSize = { 0,0 };
static ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
static entt::entity nullentity = entt::null;
static std::unordered_map<size_t, bool> DrawFullComponentData = {};
static std::unordered_map<uint32_t, const char*> iconLookup = {};
const char* GetIcon(uint32_t type)
{
	if (iconLookup.contains(type))
		return iconLookup[type];
	else
		return iconLookup[0];
}

InspectorWindow::InspectorWindow() : GuiWindow("Inspector"), inspectedEntity(nullentity)
{
	gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
	dockSpaceWindow->PushWindowMenu("Inspector Window", "", &m_isActive);
	onEntitySelected = new gns::EventFunction<void, entt::entity>([&](entt::entity selectedEntity)
	{
		LOG_INFO(static_cast<uint64_t>(selectedEntity));
		inspectedEntity = gns::Entity(selectedEntity);
		DrawFullComponentData.clear();
	});
	SelectionManager::onSelectionChanged.Subscribe<void, entt::entity>(onEntitySelected);
	iconLookup[0] = ICON_MD_CODE;
	iconLookup[entt::type_hash<gns::Camera>::value()] = ICON_MD_VIDEOCAM;
	iconLookup[entt::type_hash<gns::EntityComponent>::value()] = ICON_MD_LABEL;
	iconLookup[entt::type_hash<gns::Transform>::value()] = ICON_MD_TRANSFORM;
	iconLookup[entt::type_hash<gns::RendererComponent>::value()] = ICON_MD_CATEGORY;
}

void InspectorWindow::OnBeforeWindowDraw()
{
	GuiWindow::OnBeforeWindowDraw();
}

void InspectorWindow::OnGUI()
{
	ChildSize.y = 0;
	if(!inspectedEntity.IsValid()) return;

	ImGui::PushFont(gns::gui::GuiSystem::boldFont);
	ImGui::Text(inspectedEntity.GetComponent<gns::EntityComponent>().name.c_str());
	ImGui::PopFont();

	const std::vector<gns::ComponentMetadata>& components = inspectedEntity.GetAllComponent();
	for(const auto& component : components)
	{
		DrawComponent(component.data, component.typehash);
	}
}

void InspectorWindow::OnAfterWindowDraw()
{
	GuiWindow::OnAfterWindowDraw();
}

void InspectorWindow::DrawComponent(void* component, size_t typeHash)
{
	if(!DrawFullComponentData.contains(typeHash))
	{
		DrawFullComponentData[typeHash] = true;
	}
	ImGui::BeginChild(typeHash, ChildSize, child_flags);
	ImGui::PushFont(gns::gui::GuiSystem::boldFont);
	ImGui::Text("%s %s", GetIcon(typeHash), Serializer::ComponentData_Table[typeHash].name.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() - 80);
	if(ImGui::Button(ICON_MD_ARROW_DROP_DOWN))
		DrawFullComponentData[typeHash] = !DrawFullComponentData[typeHash];

	ImGui::SameLine(ImGui::GetWindowWidth() - 40);
	ImGui::Button(ICON_MD_DELETE_FOREVER);

	ImGui::Separator();
	ImGui::PopFont();
	if(DrawFullComponentData[typeHash])
	{
		ImGui::BeginTable(name.c_str(), 2, table_flags);
		for (size_t i =0; i < Serializer::ComponentData_Table[typeHash].fields.size(); i++)
		{
			DrawField(static_cast<char*>(component), Serializer::ComponentData_Table[typeHash].fields[i]);
		}
		/* 
		DrawField(120, &dummyfloat, "Position");
		DrawField(121, &dummyfloat, "Rotation");
		DrawField(122, &dummyfloat, "Scale");
		*/

		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void InspectorWindow::DrawField(size_t typeId, void* valuePtr, std::string& name)
{
	std::string fieldName = "##" + name;
	ImGui::TableNextRow();
	ImGui::TableNextColumn(); ImGui::Text(name.c_str());
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-1);
	DrawValue(typeId, valuePtr, name);
	ImGui::PopItemWidth();
}

void InspectorWindow::DrawField(char* componentPtr, FieldData fieldData)
{
		DrawField(fieldData.typeID, (void*)(componentPtr + fieldData.offset), fieldData.name);
}

void InspectorWindow::DrawValue(size_t typeId, void* valuePtr, std::string& name)
{
	if (typeId == typeid(glm::vec3).hash_code())
	{
		ImGui::DragFloat3(name.c_str(), static_cast<float*>(valuePtr), DragValueSensitivity);
		return;
	}
	if(typeId == typeid(float).hash_code())
	{
		ImGui::DragFloat(name.c_str(), static_cast<float*>(valuePtr), DragValueSensitivity);
		return;

	}
	if (typeId == typeid(std::string).hash_code())
	{
		ImGui::Text(static_cast<std::string*>(valuePtr)->c_str());
		return;

	}
	if (typeId == typeid(gns::rendering::Mesh).hash_code())
	{
		gns::rendering::Mesh** mesh = static_cast<gns::rendering::Mesh**>(valuePtr);
		gns::rendering::Mesh* m = *mesh;
		//std::string name = mesh->m_subMeshes[0]->name;
		ImGui::PushItemWidth(-1.0f);
		ImGui::Button(m->m_name.c_str(), {ImGui::GetContentRegionAvail().x,0});
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
			{
				LOG_INFO("Dropped into Field");
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopItemWidth();
		return;
	}
	if (typeId == typeid(gns::core::guid).hash_code())
	{
		gns::core::guid guid = *static_cast<gns::core::guid*>(valuePtr);
		ImGui::PushItemWidth(-1.0f);
		ImGui::Button(std::to_string(guid).c_str(), { ImGui::GetContentRegionAvail().x,0 });
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
			{
				LOG_INFO("Dropped into Field");
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopItemWidth();
		return;
	}
	if (typeId == typeid(std::vector<gns::rendering::Material*>).hash_code())
	{
		ImGui::PushItemWidth(-1.0f);
		std::vector<gns::rendering::Material*> materials = *static_cast<std::vector<gns::rendering::Material*>*>(valuePtr);
		for (size_t i = 0; i < materials.size(); i++ )
		{
			ImGui::Button(materials[i]->name.c_str(), {ImGui::GetContentRegionAvail().x,0});
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
				{
					LOG_INFO("Dropped into Field");
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::PopItemWidth();
		return;
	}
}
