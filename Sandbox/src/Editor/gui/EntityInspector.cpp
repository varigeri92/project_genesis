#include "EntityInspector.h"

#include <genesis.h>

#include "EditorGUI.h"
#include "Log.h"
#include "ImGui/imgui.h"

static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth;
ImGuiTreeNodeFlags node_flags = base_flags;

void gns::editor::EntityInspector::OnGUI()
{
	if (!is_entitySelected) return;

	EntityComponent& entityComponent = m_scene->registry.get<EntityComponent>(m_entity);
	ImGui::Text(entityComponent.name.c_str());
	ImGui::Separator();
	ImGui::Text("Transform:");
	Transform& transform = m_scene->registry.get<Transform>(m_entity);
	ImGui::DragFloat3("Position", (float*)&transform.position, 0.01f);
	ImGui::DragFloat3("Rotation", (float*)&transform.rotation, 0.01f);
	ImGui::DragFloat3("Scale", (float*)&transform.scale, 0.01f);
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
}

void gns::editor::EntityInspector::SetInspectedEntity(entt::entity entity, Scene* scene)
{
	m_scene = scene;
	m_entity = entity;
	auto reg = m_scene->registry.storage();
	for (auto&& curr : reg)
	{
		entt::id_type id = curr.first;

		if (auto& storage = curr.second; storage.contains(entity))
		{
			std::cout << "Entity has component id = " << id << std::endl;
			
		}
	}
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
