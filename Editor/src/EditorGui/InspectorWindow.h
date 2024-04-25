#pragma once
#include "engine.h"

class InspectorWindow : public gns::gui::GuiWindow
{
public:
	InspectorWindow();

protected:
	void OnBeforeWindowDraw() override;
	void OnGUI() override;
	void OnAfterWindowDraw() override;

private:
	void DrawComponent(std::string name);
	void DrawField(size_t typeId, void* valuePtr, std::string name);
};

