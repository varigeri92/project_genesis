#include "EditorGUI.h"
#include "EntityInspector.h"
#include "EnTT/entt.hpp"
#include "ImGui/imgui.h"
#include "Log.h"
#include <genesis.h>

static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth;
ImGuiTreeNodeFlags node_flags = base_flags;


void DrawProperty(SerializedProperty* property)
{
	if(property->type == "bool")
	{
		ImGui::Checkbox(property->name.c_str(), static_cast<bool*>(property->data));
	}

	if (property->type == "std::string")
	{
		ImGui::Text(property->name.c_str());
	}
	if (property->type == "glm::vec3")
	{
		ImGui::DragFloat3(property->name.c_str(), static_cast<float*>(property->data), 0.01f);
	}
	if (property->type == "float")
	{
		ImGui::DragFloat(property->name.c_str(), static_cast<float*>(property->data), 0.01f);
	}
}

void DrawComponent(const gns::ComponentBase* component)
{
	for(const auto property : component->serializedProperties)
	{
		DrawProperty(property);
	}
}

void gns::editor::EntityInspector::OnGUI()
{
	if (!is_entitySelected) return;
	/*
	EntityComponent& entityComponent = m_scene->registry.get<EntityComponent>(m_entity);
	DrawComponent(&entityComponent);
	Transform& transform = m_scene->registry.get<Transform>(m_entity);
	DrawComponent(&transform);
	transform.UpdateMatrix();
	ImGui::Separator();

	RendererComponent* rendererComponent = m_scene->registry.try_get<RendererComponent>(m_entity);
	if(rendererComponent != nullptr)
	{
		ImGui::Text("Renderer");
	}

	CameraComponent* cameraComponent = m_scene->registry.try_get<CameraComponent>(m_entity);
	if (cameraComponent != nullptr)
	{
		ImGui::Text("Camera");
	}
*/

	for (ComponentMetadata& component: Entity::ComponentRegistry[m_entity])
	{
		ImGui::Text(component.name.c_str());
		DrawComponent((ComponentBase*)(component.data));
		ImGui::Separator();
	}
	
}

void gns::editor::EntityInspector::SetInspectedEntity(entt::entity entity, Scene* scene)
{
	m_scene = scene;
	m_entity = entity;
	/* 
	auto reg = m_scene->registry.storage();
	for (auto&& curr : reg)
	{
		entt::id_type id = curr.first;

		if (auto& storage = curr.second; storage.contains(entity))
		{
			std::cout << "Entity has component id = " << storage.type().hash() << " the typename is: "
			<< storage.type().name() << std::endl;
		}
	}
	*/
	is_entitySelected = true;
}


gns::editor::SceneHierarchy::SceneHierarchy() :
	GuiWindow("Scene Hierarchy")
{
	m_entityInspector = dynamic_cast<EntityInspector*>(EditorGUI::GetWindow("Inspector"));
	if(m_entityInspector == nullptr)
	{
		LOG_ERROR("'Inspector' Window Does not exists!!");
	}
	m_scene = SceneManager::GetActiveScene();
}

void gns::editor::SceneHierarchy::OnGUI()
{
	if (ImGui::CollapsingHeader(m_scene->name.c_str(), &m_headerOpen, ImGuiTreeNodeFlags_None))
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		auto entityView = m_scene->registry.view<EntityComponent>();
		for (auto [entt, entity] : entityView.each())
		{
			bool node_open = ImGui::TreeNodeEx((void*)entt, node_flags, entity.name.c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				LOG_INFO("Clicked on: " << entity.name);
				m_entityInspector->SetInspectedEntity(entt, m_scene);
			}
			if(node_open)
			{
				
			}
		}
	}
}
