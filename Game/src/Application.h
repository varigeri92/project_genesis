#pragma once
#include "api.h"
#include "Engine.h"

namespace gns::application
{
	class Editor
	{
		Application_API Editor() = default;
		Application_API ~Editor() = default;

		Application_API Editor(const Editor& other) = delete;
		Application_API Editor operator=(const Editor& other) = delete;
	};
}