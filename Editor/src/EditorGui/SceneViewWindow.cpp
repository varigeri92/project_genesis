#include "SceneViewWindow.h"
#include "DockspaceWindow.h"
#include "Engine.h"
#include "../DragDropManager.h"
#include "../../../Engine/src/AssetDatabase/AssetLoader.h"
#include "../../../Engine/src/Gui/ImGui/imgui_internal.h"
#include "../../../Engine/src/Level/SceneManager.h"
#include "../AssetManager/AssetImporter.h"
#include "../Utils/Utilities.h"

gns::editor::SceneViewWindow::SceneViewWindow(const std::string& name): GuiWindow(name), m_WindowInitialized(false)
{
	rendering::OffscreenPass& offscreenPass = SystemsAPI::GetSystem<RenderSystem>()->GetOffscreenPass();
	m_Dset = ImGui_ImplVulkan_AddTexture(offscreenPass.sampler, offscreenPass.color.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_renderSystem = SystemsAPI::GetSystem<RenderSystem>();
	DockspaceWindow* dockSpaceWindow = gui::GuiSystem::GetWindow<DockspaceWindow>();
	dockSpaceWindow->PushWindowMenu("Scene View", "", &m_isActive);
}

gns::editor::SceneViewWindow::~SceneViewWindow()
{
	
}

void gns::editor::SceneViewWindow::OnGUI()
{
	varPopped = true;
	ImGui::PopStyleVar(1);
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	
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
	if(IsWindowResized(viewportPanelSize) && m_WindowInitialized)
	{
		LOG_INFO("Viewport resized! >> " << viewportPanelSize.x << " / "<<viewportPanelSize.y);
		//m_renderSystem->RecreateFrameBuffer(viewportPanelSize.x, viewportPanelSize.y);
		//Screen::SetResolution(viewportPanelSize.x, viewportPanelSize.y);
		CameraSystem* cameraSystem = SystemsAPI::GetSystem<CameraSystem>();
		cameraSystem->UpdateProjection(viewportPanelSize.x, viewportPanelSize.y);
	}
	m_WindowInitialized = true;
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
