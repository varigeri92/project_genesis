#pragma once
#include "GUI/GuiWindow.h"

namespace gns::editor
{
	class EditorGUI
	{
	public:
		static gui::GuiWindow* GetWindow(std::string name);
	};
}

