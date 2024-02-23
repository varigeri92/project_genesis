#pragma once
#include "genesis.h"
#include "GUI/GuiWindow.h"

namespace gns::gui {

class TexturePreviewWindow : public GuiWindow
{
public:
	std::vector<std::shared_ptr<gns::rendering::Texture>> texturesToShow;
	size_t Current;

	TexturePreviewWindow();
	// Inherited via GuiWindow
	GNS_API void OnGUI() override;

	void AddTexture(std::shared_ptr<gns::rendering::Texture> texture);
};
}
