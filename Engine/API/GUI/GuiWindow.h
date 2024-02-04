#pragma once
#include <string>
#include <gnsAPI.h>

#include "ImGui/imgui.h"

namespace gns
{
	class GUI;
}

namespace gns::gui
{
	class GuiWindow
	{
	friend class gns::GUI;
	private:
		void DrawWindow();
	protected:
		bool m_isFocused;
		bool m_isOpen;
		ImGuiWindowFlags_ m_flags = ImGuiWindowFlags_None;
		GEN_API virtual void OnGUI() = 0;
	public:
	virtual ~GuiWindow() = default;
	GuiWindow(std::string name);
		std::string Name;
	};
}
