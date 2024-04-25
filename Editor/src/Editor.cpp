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
        });
    engine->Run();
    engine->ShutDown();
}
