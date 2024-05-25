#pragma once
#include "engine.h"
struct FieldData;
namespace gns::editor
{
	class InspectorWindow : public gns::gui::GuiWindow
	{
	public:
		InspectorWindow();
		~InspectorWindow();
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

		void DrawMaterials(gns::RendererComponent* rendererComponent);
		void DrawMaterial(gns::rendering::Material* material, size_t index);
		void DrawMaterialAttribute(gns::rendering::Shader::AttributeInfo info,
			gns::rendering::Material* material, size_t index);

		void DropToField();

		gns::EventFunction<void, entt::entity>* onEntitySelected;
		gns::Entity inspectedEntity;
	};
}