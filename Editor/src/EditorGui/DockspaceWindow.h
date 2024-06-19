#pragma once
#include "Engine.h"

namespace gns::editor
{
	struct MenuItem
	{
		std::string label;
		std::string shortcut;
		bool* windowOpen_ptr;
	};
	class DockspaceWindow final : public gns::gui::GuiWindow
	{
	public:
		DockspaceWindow();
		~DockspaceWindow() = default;
		void PushWindowMenu(std::string label, std::string shortCut, bool* windowOpen);
	protected:
		void OnGUI() override;
		void OnBeforeWindowDraw() override;
		void BuildWindowMenu() const;

		std::vector<MenuItem> m_windowItems;
	};
};
