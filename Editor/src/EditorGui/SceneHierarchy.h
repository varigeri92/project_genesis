#pragma once
#include "Engine.h"

namespace gns::editor
{
	class SceneHierarchy final : public gns::gui::GuiWindow
	{
	public:
		SceneHierarchy();

	protected:
		//void OnBeforeWindowDraw() override;
		void OnGUI() override;
		//void OnAfterWindowDraw() override;
	public:
	};
}
