#include <iostream>
#include "GenAPI.h"
#include "Log.h"
#include "Application.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "AssetDatabase/tiny_obj_loader.h"

using namespace gns;
GEN_API int CreateEngine()
{
	LOG_INFO("Hello Genesis engine");
	Application* application = new Application();
	application->Run();
	delete(application);
	return 0;
}

