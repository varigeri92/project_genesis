#pragma once
#include "gnsAPI.h"
#include "../src/Application.h"

GNS_API gns::Application* CreateApplication(const char* WorkingDirectory);
GNS_API void InitClientData();
GNS_API void RunMainLoop();