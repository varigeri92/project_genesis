#pragma once
#include "Engine.h"

namespace gns::editor
{
	class ContentBrowserWindow final : public gns::gui::GuiWindow
	{
	public:
		ContentBrowserWindow();

	protected:
		//void OnBeforeWindowDraw() override;
		void OnGUI() override;
		//void OnAfterWindowDraw() override;
	public:
	};
}
