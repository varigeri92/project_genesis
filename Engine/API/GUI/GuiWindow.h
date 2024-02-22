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
		GNS_API virtual void OnGUI() = 0;

	public:

		std::string name;
		size_t index;
		size_t typeHash;
		bool m_isActive = true;
		std::string Name;

		virtual ~GuiWindow() = default;
		GNS_API GuiWindow(std::string name);
		GNS_API void SetActive(bool value);
	};

}
