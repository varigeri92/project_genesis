#include "TexturePreviewWindow.h"

ImVec2 size = { 512,512 };

gns::gui::TexturePreviewWindow::TexturePreviewWindow() : GuiWindow("Texture Preview"), Current (0)
{}

void gns::gui::TexturePreviewWindow::OnGUI()
{
    ImGui::Text("HELLO GUYS!");
}

void gns::gui::TexturePreviewWindow::AddTexture(std::shared_ptr<gns::rendering::Texture> texture)
{
    texturesToShow.push_back(texture);
}
