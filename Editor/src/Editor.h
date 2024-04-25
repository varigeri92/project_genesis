#pragma once
#include "api.h"
#include "Engine.h"

namespace gns::editor
{
	class Editor
	{
	public:
		Engine* engine;
		Editor() = delete;
		Editor_API Editor(Engine* engine);
		Editor_API ~Editor() = default;

		Editor_API Editor(const Editor& other) = delete;
		Editor_API Editor operator=(const Editor& other) = delete;

	};
}