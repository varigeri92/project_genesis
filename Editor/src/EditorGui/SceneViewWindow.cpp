#include "SceneViewWindow.h"

#include "DockspaceWindow.h"
#include "Engine.h"
#include "../../../Engine/src/Gui/ImGui/imgui_internal.h"

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
