#include "InspectorWindow.h"
#include "../../../Engine/src/Gui/ImGui/IconsMaterialDesign.h"
#include "../SelectionManager.h"
#include "DockspaceWindow.h"
#include "../DragDropManager.h"
#include "../../../Engine/src/Gui/ImGui/imgui_stdlib.h"
using namespace gns::gui;
namespace gns::editor
{
	static std::shared_ptr<gns::rendering::Texture> PlaceholderButtonIcon;
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit;
	static ImVec2 ChildSize = { 0,0 };
	static ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle;
	static entt::entity nullentity = entt::null;
	bool drawMaterial;
	static std::unordered_map<size_t, bool> DrawFullComponentData = {};
	static std::unordered_map<uint32_t, const char*> iconLookup = {};
	const char* GetIcon(uint32_t type)
	{
		if (iconLookup.contains(type))
			return iconLookup[type];
		else
			return iconLookup[0];
	}
	bool showAddComponentMenu = false;
	InspectorWindow::InspectorWindow() : GuiWindow("Inspector"), inspectedEntity(nullentity)
	{
		drawMaterial = true;
		gns::editor::DockspaceWindow* dockSpaceWindow = gns::gui::GuiSystem::GetWindow<gns::editor::DockspaceWindow>();
		dockSpaceWindow->PushWindowMenu("Inspector Window", "", &m_isActive);
		onEntitySelected = new gns::EventFunction<void, entt::entity>([&](entt::entity selectedEntity)
			{
				inspectedEntity = gns::Entity(selectedEntity);
				DrawFullComponentData.clear();
				showAddComponentMenu = false;
			});
		SelectionManager::onSelectionChanged.Subscribe<void, entt::entity>(onEntitySelected);
		iconLookup[0] = ICON_MD_CODE;
		iconLookup[entt::type_hash<gns::Camera>::value()] = ICON_MD_VIDEOCAM;
		iconLookup[entt::type_hash<gns::EntityComponent>::value()] = ICON_MD_LABEL;
		iconLookup[entt::type_hash<gns::Transform>::value()] = ICON_MD_TRANSFORM;
		iconLookup[entt::type_hash<gns::RendererComponent>::value()] = ICON_MD_CATEGORY;

		PlaceholderButtonIcon = std::make_shared<gns::rendering::Texture>(R"(Icons\Material_Icon.png)", true);
		PlaceholderButtonIcon->Apply();
		PlaceholderButtonIcon->m_descriptorSet = ImGui_ImplVulkan_AddTexture(PlaceholderButtonIcon->m_sampler, PlaceholderButtonIcon->m_imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	InspectorWindow::~InspectorWindow()
	{
		PlaceholderButtonIcon->Destroy();
	}

	void InspectorWindow::OnBeforeWindowDraw()
	{
		GuiWindow::OnBeforeWindowDraw();
	}

	using AttributeType = gns::rendering::Shader::ShaderAttributeType;

	gns::RendererComponent* rendererComponent = nullptr;
	void InspectorWindow::OnGUI()
	{
		ChildSize.y = 0;
		

		if (!inspectedEntity.IsValid())
		{
			if(!SelectionManager::isSelectionImported)
			{
				ImGui::Text("Selected Asset is not imported!");
				if(ImGui::Button("Import", { ImGui::GetContentRegionAvail().x,20 }))
				{
					LOG_INFO("Importing asset: xxx");
				}
			}
			return;
		}
			

		ImGui::PushFont(gns::gui::GuiSystem::boldFont);
		ImGui::Text("%i; %s",inspectedEntity.entity, inspectedEntity.GetComponent<gns::EntityComponent>().name.c_str());
		ImGui::PopFont();

		const std::vector<gns::ComponentMetadata>& components = inspectedEntity.GetAllComponent();
		for (const auto& component : components)
		{
			DrawComponent(component.data, component.typehash);
		}

		ImGui::Spacing();
		if(ImGui::Button("Add Component...", { ImGui::GetContentRegionAvail().x, 35 }))
		{
			showAddComponentMenu = !showAddComponentMenu;
		}
		if(showAddComponentMenu)
		{
			ImGui::BeginChild("add_cmp_menu");
			for (auto it = Serializer::ComponentData_Table.begin(); it != Serializer::ComponentData_Table.end(); ++it) {
				if(ImGui::Button(it->second.name.c_str()))
				{
					LOG_INFO("AddComponent:" << it->second.name);
					Serializer serilizer;
					serilizer.AddComponentByID(it->second.typeID, inspectedEntity);
					showAddComponentMenu = false;
				}
			}

			if (ImGui::Button("Cancel"))
			{
				showAddComponentMenu = false;
			}
			ImGui::EndChild();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		if (inspectedEntity.TryGetComponent<gns::RendererComponent>(rendererComponent))
		{
			rendererComponent = &inspectedEntity.GetComponent<gns::RendererComponent>();
			DrawMaterials(rendererComponent);
		}
	}

	void InspectorWindow::OnAfterWindowDraw()
	{
		GuiWindow::OnAfterWindowDraw();
	}

	void InspectorWindow::DrawComponent(void* component, size_t typeHash)
	{
		if (!DrawFullComponentData.contains(typeHash))
		{
			DrawFullComponentData[typeHash] = true;
		}
		ImGui::BeginChild(typeHash, ChildSize, child_flags);
		ImGui::PushFont(gns::gui::GuiSystem::boldFont);
		ImGui::Text("%s %s", GetIcon(typeHash), Serializer::ComponentData_Table[typeHash].name.c_str());

		ImGui::SameLine(ImGui::GetWindowWidth() - 80);
		if (ImGui::Button(ICON_MD_ARROW_DROP_DOWN))
			DrawFullComponentData[typeHash] = !DrawFullComponentData[typeHash];

		ImGui::SameLine(ImGui::GetWindowWidth() - 40);
		ImGui::Button(ICON_MD_DELETE_FOREVER);

		ImGui::Separator();
		ImGui::PopFont();
		if (DrawFullComponentData[typeHash])
		{
			std::string tableID = name + "__";
			if (ImGui::BeginTable(tableID.c_str(), 2, table_flags))
			{
				for (size_t i = 0; i < Serializer::ComponentData_Table[typeHash].fields.size(); i++)
				{
					DrawField(static_cast<char*>(component), Serializer::ComponentData_Table[typeHash].fields[i]);
				}
				ImGui::EndTable();
			}
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
	char buffer[255];
	void InspectorWindow::DrawValue(size_t typeId, void* valuePtr, std::string& name)
	{
		if (typeId == typeid(glm::vec3).hash_code())
		{
			ImGui::DragFloat3(name.c_str(), static_cast<float*>(valuePtr), DragValueSensitivity);
			return;
		}
		if (typeId == typeid(float).hash_code())
		{
			ImGui::DragFloat(name.c_str(), static_cast<float*>(valuePtr), DragValueSensitivity);
			return;

		}
		if (typeId == typeid(std::string).hash_code())
		{
			ImGui::InputText(name.c_str(), static_cast<std::string*>(valuePtr));
			return;

		}

		if (typeId == typeid(Color).hash_code())
		{
			ImGui::ColorEdit4(name.c_str(), static_cast<float*>(valuePtr));
			return;

		}

		if (typeId == typeid(gns::rendering::Mesh).hash_code())
		{
			gns::rendering::Mesh** mesh = static_cast<gns::rendering::Mesh**>(valuePtr);
			gns::rendering::Mesh* m = *mesh;
			//std::string name = mesh->m_subMeshes[0]->name;
			ImGui::PushItemWidth(-1.0f);
			ImGui::Text(std::to_string(m->GetGuid()).c_str());
			ImGui::Button(m->m_name.c_str(), { ImGui::GetContentRegionAvail().x,0 });
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
			ImGui::PushItemWidth(0.0f);
			std::vector<gns::rendering::Material*> materials = *static_cast<std::vector<gns::rendering::Material*>*>(valuePtr);
			for (size_t i = 0; i < materials.size(); i++)
			{
				ImGui::Button(materials[i]->name.c_str(), { ImGui::GetContentRegionAvail().x,0 });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
					{
						DropToField();
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::PopItemWidth();
			return;
		}
	}

	void InspectorWindow::DrawMaterials(gns::RendererComponent* rendererComponent)
	{

		ImGui::BeginChild("Material Drawer", ChildSize, child_flags);

		ImGui::PushFont(gns::gui::GuiSystem::boldFont);
		ImGui::Text("%s %s", GetIcon(0), "Materials:");

		ImGui::SameLine(ImGui::GetWindowWidth() - 40);
		if (ImGui::Button(ICON_MD_ARROW_DROP_DOWN))
			drawMaterial = !drawMaterial;

		ImGui::Separator();
		ImGui::PopFont();
		if (!drawMaterial) {
			ImGui::EndChild();
			return;
		}
		for (size_t i = 0; i < rendererComponent->m_materials.size(); i++)
		{
			DrawMaterial(rendererComponent->m_materials[i], i);
		}
		ImGui::EndChild();
	}

	void InspectorWindow::DrawMaterial(gns::rendering::Material* material, size_t index)
	{
		ImGui::BeginChild((material->name + std::to_string(index)).c_str(), ChildSize, child_flags);
		ImGui::PushFont(gns::gui::GuiSystem::boldFont);
		ImGui::Text("%s %s", GetIcon(0), material->name.c_str());
		ImGui::Separator();
		ImGui::PopFont();

		ImGui::SeparatorText("Shaders:");
		if (ImGui::Button("V.Shader-name", { ImGui::GetContentRegionAvail().x,0 }))
		{
			LOG_INFO("open shgaer selection menu...");
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
			{
				DropToField();
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::Button("F.Shader-name", { ImGui::GetContentRegionAvail().x,0 }))
		{
			LOG_INFO("open shgaer selection menu...");
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
			{
				DropToField();
			}
			ImGui::EndDragDropTarget();
		}
		if(ImGui::Button("Rebuild Pipeline"))
		{
			SystemsAPI::GetSystem<RenderSystem>()->CreatePipeline(material->m_shader);
		}

		ImGui::SeparatorText("Material Properties:");
		std::string tableID = "Table_" + std::to_string(index);
		if (ImGui::BeginTable(tableID.c_str(), 2, table_flags))
		{
			for (size_t i = 0; i < material->m_shader->m_fragmentShaderAttributes.size(); ++i)
			{
				DrawMaterialAttribute(material->m_shader->m_fragmentShaderAttributes[i], material, index);
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	void InspectorWindow::DrawMaterialAttribute(gns::rendering::Shader::AttributeInfo info,
		gns::rendering::Material* material, size_t index)
	{
		std::string fieldName = "##" + info.attributeName + std::to_string(index);
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); ImGui::Text(info.attributeName.c_str());
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-1);
		switch (info.type)
		{
		case gns::rendering::Shader::ShaderAttributeType::Float:
			ImGui::DragFloat(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName));
			break;
		case gns::rendering::Shader::ShaderAttributeType::Float2:
			ImGui::DragFloat2(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName));
			break;
		case gns::rendering::Shader::ShaderAttributeType::Float3:
			ImGui::DragFloat3(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName));
			break;
		case gns::rendering::Shader::ShaderAttributeType::Float4:
			ImGui::DragFloat4(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName));
			break;
		case gns::rendering::Shader::ShaderAttributeType::Mat4:
			break;
		case gns::rendering::Shader::ShaderAttributeType::Color:
			ImGui::ColorEdit3(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName),
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
			break;
		case gns::rendering::Shader::ShaderAttributeType::Color4:
			ImGui::ColorEdit4(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName),
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
			break;
		case gns::rendering::Shader::ShaderAttributeType::HdrColor4:
			ImGui::ColorEdit4(fieldName.c_str(), material->GetFragmentShaderAttribute<float>(info.attributeName),
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_HDR);
			break;
		case gns::rendering::Shader::ShaderAttributeType::Int:
			ImGui::DragInt(fieldName.c_str(), material->GetFragmentShaderAttribute<int>(info.attributeName));
			break;
		case gns::rendering::Shader::ShaderAttributeType::Flag:
			break;
		case gns::rendering::Shader::ShaderAttributeType::Texture:
			ImGui::ImageButton(fieldName.c_str(), PlaceholderButtonIcon->m_descriptorSet, { 45,45 });
			break;
		default:;
		}
		ImGui::PopItemWidth();
	}

	void InspectorWindow::DropToField()
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
		{
			LOG_INFO("Dragdrop operation accepted! source: " << gns::editor::DragDropManager::GetCurrentPayload()->path);
		}
	}
}