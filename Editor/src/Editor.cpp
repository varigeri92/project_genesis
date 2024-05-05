#include "Editor.h"
#include "Engine.h"
#include "api.h"
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
            gui::GuiSystem::RegisterWindow<DockspaceWindow>("DockSpace");
            gui::GuiSystem::RegisterWindow<SceneViewWindow>("Scene");
            gui::GuiSystem::RegisterWindow<DummyWindow>("Dummy");
            gui::GuiSystem::RegisterWindow<InspectorWindow>();
            gui::GuiSystem::RegisterWindow<ContentBrowserWindow>();
            gui::GuiSystem::RegisterWindow<SceneHierarchy>();

            Entity cameraEntity = Entity::CreateEntity("Editor_Camera");
            Transform& cameraTransform = cameraEntity.GetComponent<Transform>();
            Camera& cam = cameraEntity.AddComponet<Camera>(0.01, 1000, 60, 1920, 1080, cameraTransform);
            CameraSystem* cameraSystem = SystemsAPI::RegisterSystem<CameraSystem>();

            cameraTransform.position = { 0.f,-1.f,-5.f };
            cameraSystem->UpdateProjection(1920, 1080);

        });
    engine->Run();
    engine->ShutDown();
}
