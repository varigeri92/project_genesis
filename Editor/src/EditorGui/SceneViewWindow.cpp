#include "SceneViewWindow.h"
#include "DockspaceWindow.h"
#include "Engine.h"
#include "../DragDropManager.h"
#include "../SelectionManager.h"
#include "../AssetManager/AssetImporter.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../EditorCamera.h"

gns::Transform* currentEntityTransform;
entt::entity selected = entt::null;
EditorCamera* camera = nullptr;
gns::editor::SceneViewWindow::SceneViewWindow() : GuiWindow("Scene View"), m_WindowInitialized(false), m_isGizmoLocal(true)
{
	m_renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	m_offscreenPass = &m_renderSystem->GetOffscreenPass();
	m_Dset = ImGui_ImplVulkan_AddTexture(m_offscreenPass->descriptor.sampler, m_offscreenPass->descriptor.imageView,
		m_offscreenPass->descriptor.imageLayout);
	DockspaceWindow* dockSpaceWindow = gui::GuiSystem::GetWindow<DockspaceWindow>();
	dockSpaceWindow->PushWindowMenu("Scene View", "", &m_isActive);
	camera = SystemsAPI::GetSystem<EditorCamera>();
	onEntitySelected = new gns::EventFunction<void, entt::entity>([&](entt::entity selectedEntity)
		{
			selected = selectedEntity;
			if(selected!= entt::null)
				currentEntityTransform = &gns::Entity(selectedEntity).GetComponent<Transform>();
		});

	SelectionManager::onSelectionChanged.Subscribe<void, entt::entity>(onEntitySelected);
	m_gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	m_gizmoMode = ImGuizmo::MODE::WORLD;
}

gns::editor::SceneViewWindow::~SceneViewWindow()
{
	
}
void decomposeMatrix(const glm::mat4& matrix, glm::vec3& scale, glm::quat& rotation, glm::vec3& translation) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, translation, skew, perspective);
}
ImVec2 viewportPanelSize = {0,0};
void gns::editor::SceneViewWindow::OnGUI()
{
	varPopped = true;
	ImGui::PopStyleVar(1);
	viewportPanelSize = ImGui::GetContentRegionAvail();
	viewportPanelSize.y -= ImGui::GetTextLineHeightWithSpacing() + 5;
	if (ImGui::Button("L/W"))
	{
		if(m_gizmoMode == ImGuizmo::MODE::LOCAL)
		{
			m_gizmoMode = ImGuizmo::MODE::WORLD;
		}
		else
		{
			m_gizmoMode = ImGuizmo::MODE::LOCAL;
		}

	}ImGui::SameLine();
	if (ImGui::Button("T"))
	{
		m_gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	}ImGui::SameLine();
	if (ImGui::Button("R"))
	{
		m_gizmoOperation = ImGuizmo::OPERATION::ROTATE;
	}ImGui::SameLine();
	if (ImGui::Button("S"))
	{
		m_gizmoOperation = ImGuizmo::OPERATION::SCALE;
	}ImGui::SameLine();
	if (ImGui::Button("B"))
	{
		m_gizmoOperation = ImGuizmo::OPERATION::BOUNDS;
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
			std::string path = DragDropManager::GetCurrentPayload()->path;
			std::string extension = fileSystem::FileSystem::GetFileExtension(path);
			if (extension == "gnsscene") {
				core::SceneManager::loadSceneFromFile(path);
			}
			else 
			{
				AssetImporter importer;
				AssetMetadata assetMeta = {};
				if(importer.ImportAsset(path, assetMeta))
				{
					LOG_INFO("Asset imported sucessfully, or it was already imported... guid:" << assetMeta.guid);
					AssetMetadata importedAsset = AssetDatabase::AddImportedAssetToDatabase(assetMeta);
					auto* mesh = AssetLoader::LoadAssetFromFile<rendering::Mesh>(importedAsset.guid);
					SystemsAPI::GetSystem<RenderSystem>()->UploadMesh(mesh);
					Entity entity = core::SceneManager::ActiveScene->CreateEntity(gns::fileSystem::FileSystem::GetFileName(importedAsset.sourcePath));
					entity.AddComponet <RendererComponent>(importedAsset.guid, 0);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	m_WindowInitialized = true;
	if(selected == entt::null) return;

	glm::mat4 gridMatrix = glm::mat4(1);
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetTextLineHeightWithSpacing(),
		(float)viewportPanelSize.x, (float)viewportPanelSize.y);
	glm::mat4 t_matrix = currentEntityTransform->matrix;
	glm::mat4 view = camera->m_camera.view;
	glm::mat4 projection = camera->m_camera.projection;
	projection[1][1] *= -1;

	ImGuizmo::Manipulate(reinterpret_cast<float*>(&view), reinterpret_cast<float*>(&projection),
		m_gizmoOperation, m_gizmoMode,
		reinterpret_cast<float*>(&t_matrix));

	if(ImGuizmo::IsUsing())
	{
		glm::quat rotation = {};
		decomposeMatrix(t_matrix, currentEntityTransform->scale, rotation, currentEntityTransform->position);
		currentEntityTransform->rotation = glm::eulerAngles(rotation);
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
	camera->UpdateProjection(viewportPanelSize.x, viewportPanelSize.y);
}
