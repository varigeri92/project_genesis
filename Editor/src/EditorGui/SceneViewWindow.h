#pragma once
#include "Engine.h"

namespace gns::rendering
{
	class Texture;
}

namespace gns::editor
{
class SceneViewWindow : public gns::gui::GuiWindow
{
private:
	bool varPopped = false;
	ImVec2 m_view;
	std::shared_ptr<rendering::Texture> fbTexture;
	ImTextureID m_Dset;
	rendering::RenderPass* m_offscreenPass;
	RenderSystem* m_renderSystem;

	bool m_WindowInitialized;
	gns::EventFunction<void, entt::entity>* onEntitySelected;
public:
	explicit SceneViewWindow(const std::string& name);
	~SceneViewWindow();

protected:
	void OnGUI() override;
	void OnBeforeWindowDraw() override;
	bool IsWindowResized(ImVec2 view);
	void OnAfterWindowDraw() override;
	void OnWindowResized(uint32_t width, uint32_t height);

public:
};
}
