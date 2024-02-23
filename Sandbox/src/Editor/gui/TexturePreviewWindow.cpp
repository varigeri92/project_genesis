#include "TexturePreviewWindow.h"

ImVec2 size = { 512,512 };

gns::gui::TexturePreviewWindow::TexturePreviewWindow() : GuiWindow("Texture Preview"), Current (0)
{}

void gns::gui::TexturePreviewWindow::OnGUI()
{
    int i = 0;
    for (auto Texture : texturesToShow) {
        if (ImGui::Button(std::to_string(i).c_str()))
            Current = i;
        ImGui::SameLine();
    }
    ImGui::NewLine();
    if(texturesToShow.size()>0)
        ImGui::Image(texturesToShow[Current]->descriptorSet, size);
}

void gns::gui::TexturePreviewWindow::AddTexture(std::shared_ptr<gns::rendering::Texture> texture)
{
    texturesToShow.push_back(texture);
}
