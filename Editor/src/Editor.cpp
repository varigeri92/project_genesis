#include "Editor.h"
#include "Engine.h"
#include "api.h"
#include "EditorGui/DockspaceWindow.h"
#include "EditorGui/DummyWindow.h"
#include "EditorGui/InspectorWindow.h"
#include "EditorGui/SceneViewWindow.h"

gns::editor::Editor::Editor(Engine* engine) : engine(engine)
{
    engine->Init([&]()
        {
            gui::GuiSystem::RegisterWindow<DockspaceWindow>("DockSpace");
            gui::GuiSystem::RegisterWindow<SceneViewWindow>("Scene");
            gui::GuiSystem::RegisterWindow<DummyWindow>("Dummy");
            gui::GuiSystem::RegisterWindow<InspectorWindow>();
        });
    engine->Run();
    engine->ShutDown();
}
