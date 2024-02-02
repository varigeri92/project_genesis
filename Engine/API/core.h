#pragma once
#include "gnsAPI.h"
#include "../src/Application.h"

GEN_API gns::Application* CreateApplication(const char* WorkingDirectory);
GEN_API void InitClientData();
GEN_API void RunMainLoop();