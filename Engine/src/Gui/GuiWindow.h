#pragma once
#include "../Utils/Guid.h"
#include "ImGui/imgui.h"

namespace gns::gui
{
	class GuiSystem;
}

namespace gns::gui
{
	class GuiWindow
	{
		friend class gns::gui::GuiSystem;
	private:
		void DrawWindow();

	protected:
		core::guid m_instanceId;
		bool m_isFocused;
		bool m_isOpen;
		GNS_API virtual void OnGUI() = 0;
		GNS_API virtual void OnBeforeWindowDraw();
		GNS_API virtual void OnAfterWindowDraw();
	public:
		ImGuiID m_windowID;
		ImGuiWindowFlags m_flags;
		std::string name;
		size_t index;
		size_t typeHash;
		bool m_isActive = true;

		virtual ~GuiWindow() = default;
		GNS_API GuiWindow(const std::string name);
		GNS_API void SetActive(bool value);

	};
}
