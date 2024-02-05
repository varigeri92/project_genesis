#include "EditorGUI.h"
#include <genesis.h>

gns::gui::GuiWindow* gns::editor::EditorGUI::GetWindow(std::string name)
{
	return gns::GUI::GetWindow_Internal(name);
}
