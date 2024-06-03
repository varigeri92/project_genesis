#include "Editor.h"
#include "Engine.h"
#include "api.h"
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
            Entity cameraEntity = Entity::CreateEntity("Editor_Camera", core::SceneManager::ActiveScene);
            Transform& cameraTransform = cameraEntity.GetComponent<Transform>();
            Camera& cam = cameraEntity.AddComponet<Camera>(0.01, 1000, 60, 1920, 1080, cameraTransform);
            CameraSystem* cameraSystem = SystemsAPI::RegisterSystem<CameraSystem>();

            gui::GuiSystem::RegisterWindow<DockspaceWindow>("DockSpace");
            gui::GuiSystem::RegisterWindow<SceneViewWindow>("Scene");
            gui::GuiSystem::RegisterWindow<DummyWindow>("Dummy");
            gui::GuiSystem::RegisterWindow<InspectorWindow>();
            gui::GuiSystem::RegisterWindow<ContentBrowserWindow>();
            gui::GuiSystem::RegisterWindow<SceneHierarchy>();

            cameraTransform.position = { 0.f,1.f,5.f };
            cameraSystem->UpdateProjection(1920, 1080);

            // Entity tests:
            //const std::shared_ptr<rendering::Shader> defaultShader = std::make_shared<rendering::Shader>("blinphong.vert.spv", "blinphong.frag.spv");
            rendering::Shader* defaultShader = Object::Create<rendering::Shader>("blinphong.vert.spv", "blinphong.frag.spv");
    		RenderSystem* renderSystem = SystemsAPI::GetSystem<RenderSystem>();
    		renderSystem->CreatePipeline(defaultShader);

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
            AssetDatabase::AddAssetToDatabase(assetMeta, true);
            /*
            const auto meshInstance = AssetLoader::LoadMeshFile(1,R"(sketchfab\kda_akali_-_lol_-_3d_printable_fanart.glb)");
            for (rendering::MeshData* mesh : meshInstance)
            {
                renderSystem->UploadMesh(mesh);
                Entity reiEntity = Entity::CreateEntity(mesh->name, core::SceneManager::ActiveScene);
                RendererComponent& rc = reiEntity.AddComponet<RendererComponent>(mesh, defaultMaterial);
                Transform& transform = reiEntity.GetComponent<Transform>();
                transform.position = { 0,0,0 };
                //transform.scale = { 0.25f,0.25f,0.25f };
            }
            */
        });
    engine->Run();
    engine->ShutDown();
}
