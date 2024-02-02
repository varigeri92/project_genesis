#include "core.h"
#include "Log.h"
#include "paths.h"
#include "TestClass.h"
int main()
{
    LOG_INFO("HELLO WORLD");
	gns::Application* app = CreateApplication(ASSETS_PATH);
	app->Run([](){});
}

