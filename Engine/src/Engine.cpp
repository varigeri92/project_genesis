#include <iostream>
#include "Log.h"
#include "Application.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "gnsAPI.h"
#include "AssetDatabase/tiny_obj_loader.h"

using namespace gns;
GEN_API Application* CreateApplication(const char* WorkingDirectory)
{
	LOG_INFO("Hello Genesis engine");
	Application* application = new Application(WorkingDirectory);
	return application;
}
