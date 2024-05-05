#pragma once
#include "engine.h"
struct FieldData;
class InspectorWindow : public gns::gui::GuiWindow
{
public:
	InspectorWindow();

protected:
	void OnBeforeWindowDraw() override;
	void OnGUI() override;
	void OnAfterWindowDraw() override;
	float DragValueSensitivity = 0.01f;
private:
	void DrawComponent(void* component, size_t typeHash);
	void DrawField(size_t typeId, void* valuePtr, std::string& name);
	void DrawField(char* componentPtr, FieldData fieldData);
	void DrawValue(size_t typeId, void* valuePtr, std::string& name);
	gns::EventFunction<void, entt::entity>* onEntitySelected;
	gns::Entity inspectedEntity;
};

