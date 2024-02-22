#pragma once
#include <vector>
#include "GUI/GuiWindow.h"

namespace gns::rendering
{
	class Device;
}
namespace gns
{
	class Window;
	class GUI
	{
	friend class gns::gui::GuiWindow;
	private:
		rendering::Device* m_device;
		Window* m_window;
		void InitializeGUI();

		static int32_t GetIndexOfType(size_t typeHash)
		{
			for (size_t i = 0; i < guiWindows.size(); i++)
			{
				if (guiWindows[i]->typeHash == typeHash)
				{
					return i;
				}
			}
			return -1;
		};

		static std::string GetSimpleName(std::string typeName) {
			size_t lastColonPos = typeName.rfind("::");
			if (lastColonPos != std::string::npos) {
				typeName = typeName.substr(lastColonPos + 2);
			}
			return typeName;
		}

	public:
		static GNS_API std::vector<gns::gui::GuiWindow*> guiWindows;
		GUI(rendering::Device* device, Window* window);

		template<typename T, typename... Args>
		static void RegisterWindow(Args&& ... args)
		{
			size_t typehash = typeid(T).hash_code();
			if (GetIndexOfType(typehash) == -1)
			{
				T* newWindow = new T{ std::forward<Args>(args)... };
				guiWindows.push_back(newWindow);
				size_t windowIndex = guiWindows.size() - 1;
				guiWindows[windowIndex]->index = windowIndex;
				guiWindows[windowIndex]->name = GetSimpleName(typeid(T).name());
				guiWindows[windowIndex]->typeHash = typehash;
			}
		}

		static GNS_API gns::gui::GuiWindow* GetWindow_Internal(std::string name);
		void BeginGUI();
		void EndGUI();
		void DrawGUI();
		void DisposeGUI();
	};
}

