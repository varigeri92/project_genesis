#pragma once
#include <string>
#include <gnsAPI.h>

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
		bool m_isFocused;
		bool m_isOpen;
		void DrawWindow();
	protected:
		GEN_API virtual void OnGUI() = 0;
	public:
	virtual ~GuiWindow() = default;
	GuiWindow(std::string name, GUI* gui);
		std::string Name;
	};
}
