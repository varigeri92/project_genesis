#include "../../Engine/src/AssetDatabase/AssetDatabase.h"
#include "Engine.h"
#include "../../Editor/src/Editor.h"

void SetProjectArguments(std::string command, std::string value)
{
    if( command == "-p" || command == "-project" || command ==  "-proj")
    {
        gns::AssetDatabase::SetProjectRoot(value);
    }
}

int main(int argc, char* argv[])
{
#ifdef LOG_ENABLE
    std::setvbuf(stdout, NULL, _IONBF, 0);
#endif
    
    if(argc > 1)
    {
        for (int i = 1; i < argc; i += 2) {
            SetProjectArguments(argv[i], argv[i + 1]);
        }
    }
    else
    {
        SetProjectArguments("-p", R"(D:\GenesisProjects\GenesisTestProject)");
    }

    gns::Engine* engine = new gns::Engine();
    gns::EventFunction<void, std::string>* sandboxEvent = new gns::EventFunction<void, std::string>([](const std::string& message)
        {
            LOG_INFO("External subscription: " << message);
		});
    engine->TestEvent.Subscribe<void, std::string>(sandboxEvent);
    gns::editor::Editor* editor = new gns::editor::Editor(engine);
    delete engine;
    delete editor;
}
