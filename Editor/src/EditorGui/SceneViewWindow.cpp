#include "SceneViewWindow.h"
#include "DockspaceWindow.h"
#include "Engine.h"
#include "../DragDropManager.h"
#include "../SelectionManager.h"
#include "../../../Engine/src/AssetDatabase/AssetLoader.h"
#include "../../../Engine/src/Gui/ImGui/imgui_internal.h"
#include "../../../Engine/src/Level/SceneManager.h"
#include "../AssetManager/AssetImporter.h"
#include "../Utils/Utilities.h"
#include "../../../Engine/src/Gui/ImGui/ImGuizmo.h"

gns::CameraSystem* editor_camera_System = nullptr;
gns::Transform* currentEntityTransform;
entt::entity selected = entt::null;
gns::Entity camera_entity = gns::Entity(selected);
gns::editor::SceneViewWindow::SceneViewWindow(const std::string& name): GuiWindow(name), m_WindowInitialized(false)
{
	m_renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	m_offscreenPass = &m_renderSystem->GetOffscreenPass();
	m_Dset = ImGui_ImplVulkan_AddTexture(m_offscreenPass->descriptor.sampler, m_offscreenPass->descriptor.imageView,
		m_offscreenPass->descriptor.imageLayout);
	DockspaceWindow* dockSpaceWindow = gui::GuiSystem::GetWindow<DockspaceWindow>();
	dockSpaceWindow->PushWindowMenu("Scene View", "", &m_isActive);
	//editor_camera_System = SystemsAPI::GetSystem<CameraSystem>();
	camera_entity = SystemsAPI::FindEntityOfType<Camera>();
	onEntitySelected = new gns::EventFunction<void, entt::entity>([&](entt::entity selectedEntity)
		{
			selected = selectedEntity;
			currentEntityTransform = &gns::Entity(selectedEntity).GetComponent<Transform>();
		});

	SelectionManager::onSelectionChanged.Subscribe<void, entt::entity>(onEntitySelected);
}

gns::editor::SceneViewWindow::~SceneViewWindow()
{
	
}

ImVec2 viewportPanelSize = {0,0};
void gns::editor::SceneViewWindow::OnGUI()
{
	varPopped = true;
	ImGui::PopStyleVar(1);
	viewportPanelSize = ImGui::GetContentRegionAvail();
	viewportPanelSize.y -= ImGui::GetTextLineHeightWithSpacing() + 5;
	if(ImGui::Button("Recreate Framebuffer"))
	{
		OnWindowResized(viewportPanelSize.x, viewportPanelSize.y);
	}
	if(IsWindowResized(viewportPanelSize) && m_WindowInitialized)
	{
		OnWindowResized(viewportPanelSize.x, viewportPanelSize.y);
	}
	ImGui::Image(m_Dset, ImVec2{viewportPanelSize.x, viewportPanelSize.y});
	if(ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_FILE"))
		{
			LOG_INFO("Dropped into scene!");
			AssetImporter importer;
			AssetMetadata assetMeta = {};
			if(importer.ImportAsset(DragDropManager::GetCurrentPayload()->path, assetMeta))
			{
				LOG_INFO("Asset imported sucessfully, or it was already imported... guid:" << assetMeta.guid);
				AssetMetadata importedAsset = AssetDatabase::AddAssetToDatabase(assetMeta);
				auto* mesh = AssetLoader::LoadAssetFromFile<rendering::Mesh>(importedAsset.guid);
				SystemsAPI::GetSystem<RenderSystem>()->UploadMesh(mesh);
				Entity entity = Entity::CreateEntity(utils::GetFileName(importedAsset.sourcePath), core::SceneManager::ActiveScene);
				entity.AddComponet <RendererComponent>(importedAsset.guid, 0);
			}
		}
		ImGui::EndDragDropTarget();
	}
	m_WindowInitialized = true;
	if (currentEntityTransform == nullptr) return;

	glm::mat4 gridMatrix = glm::mat4(1);
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetTextLineHeightWithSpacing(),
		(float)viewportPanelSize.x, (float)viewportPanelSize.y);
	glm::mat4 t_matrix = currentEntityTransform->matrix;
	Camera& camera = camera_entity.GetComponent<Camera>();
	glm::mat4 view = camera.view;//camera_entity.GetComponent<Transform>().matrix;
	//view = glm::inverse(view);
	glm::mat4 projection = camera.projection;
	projection[1][1] *= -1;

	/*
	ImGuizmo::DrawGrid(reinterpret_cast<float*>(&view), reinterpret_cast<float*>(&projection),
		reinterpret_cast<float*>(&gridMatrix), 25);
	 */
	if(selected == camera_entity.entity)
		return;
	ImGuizmo::Manipulate(reinterpret_cast<float*>(&view), reinterpret_cast<float*>(&projection),
		ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, 
		reinterpret_cast<float*>(&t_matrix));

	if(ImGuizmo::IsUsing())
	{
		currentEntityTransform->position = { t_matrix[3] };
	}
}

void gns::editor::SceneViewWindow::OnBeforeWindowDraw()
{
	varPopped = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	GuiWindow::OnBeforeWindowDraw();
}

bool gns::editor::SceneViewWindow::IsWindowResized(ImVec2 view)
{
	if ( view.x != m_view.x || view.y != m_view.y )
	{
		if ( view.x == 0 || view.y == 0 )
		{
			return false;
		}
		m_view.x = view.x;
		m_view.y = view.y;
		return true;
	}
	return false;
}

void gns::editor::SceneViewWindow::OnAfterWindowDraw()
{
	if(!varPopped)
		ImGui::PopStyleVar(1);
	GuiWindow::OnAfterWindowDraw();
}

void gns::editor::SceneViewWindow::OnWindowResized(uint32_t width, uint32_t height)
{
	m_renderSystem->RecreateFrameBuffer(viewportPanelSize.x, viewportPanelSize.y);
	m_Dset = ImGui_ImplVulkan_AddTexture(m_offscreenPass->descriptor.sampler, m_offscreenPass->descriptor.imageView,
		m_offscreenPass->descriptor.imageLayout);

	Screen::SetResolution(viewportPanelSize.x, viewportPanelSize.y);

	CameraSystem* cameraSystem = SystemsAPI::GetSystem<CameraSystem>();
	cameraSystem->UpdateProjection(viewportPanelSize.x, viewportPanelSize.y);
}
