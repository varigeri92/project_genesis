#include "Editor.h"
#include "Engine.h"
#include "api.h"
#include "EditorCamera.h"
#include "../../Engine/src/Level/SceneManager.h"
#include "../../Engine/src/AssetDatabase/AssetLoader.h"
#include "EditorGui/DockspaceWindow.h"
#include "EditorGui/DummyWindow.h"
#include "EditorGui/InspectorWindow.h"
#include "EditorGui/SceneViewWindow.h"
#include "EditorGui/ContentBrowserWindow.h"
#include "EditorGui/SceneHierarchy.h"


gns::editor::Editor::Editor(Engine* engine) : engine(engine)
{
    engine->Init([&]()
        {

            auto editor_camera = SystemsAPI::RegisterSystem<EditorCamera>();
            SystemsAPI::GetSystem<RenderSystem>()->SetRenderCamera(
                &editor_camera->m_camera, &editor_camera->m_transform);

            gui::GuiSystem::RegisterWindow<DockspaceWindow>("DockSpace");
            gui::GuiSystem::RegisterWindow<SceneViewWindow>("Scene");
            gui::GuiSystem::RegisterWindow<DummyWindow>("Dummy");
            gui::GuiSystem::RegisterWindow<InspectorWindow>();
            gui::GuiSystem::RegisterWindow<ContentBrowserWindow>();
            gui::GuiSystem::RegisterWindow<SceneHierarchy>();


            // Entity tests:
            //const std::shared_ptr<rendering::Shader> defaultShader = std::make_shared<rendering::Shader>("blinphong.vert.spv", "blinphong.frag.spv");
            rendering::Shader* defaultShader = Object::Create<rendering::Shader>("blinphong.vert.spv", "blinphong.frag.spv");
            const std::shared_ptr<rendering::Texture> defaultTexture = std::make_shared<rendering::Texture>(R"(Textures\uv_color_Grid.png)");

            rendering::Material* defaultMaterial = Object::Create<rendering::Material>(defaultShader, "default Material");
            defaultMaterial->SetTexture(defaultTexture, 0);
            AssetMetadata assetMeta
    		{
                defaultMaterial->GetGuid(),
                "","__default_material",
                AssetType::material,
                AssetState::loaded
            };
            AssetDatabase::AddImportedAssetToDatabase(assetMeta, true, true);
        });
    engine->Run();
    engine->ShutDown();
}
