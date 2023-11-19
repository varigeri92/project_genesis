#include <iostream>
#include "Log.h"
#include "Application.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "gnsAPI.h"
#include "AssetDatabase/tiny_obj_loader.h"

using namespace gns;
GEN_API int CreateApplication()
{
	LOG_INFO("Hello Genesis engine");
	Application* application = new Application();
	application->Run();
	delete(application);
	return 0;
}

