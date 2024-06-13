#pragma once
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_sdl2.h"
#include "ImGui/backends/imgui_impl_vulkan.h"

#include "GuiWindow.h"
#include "../SystemsApi/SystemBase.h"
#include "../SystemsApi/SystemsAPI.h"

namespace gns::rendering
{
	class Device;
}

namespace gns
{
	class Window;
}

namespace gns::gui
{
	class GuiSystem
	{
		friend class GuiWindow;
	private:
		static GNS_API bool s_showDemo;
		rendering::Device* m_device;
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
		static GNS_API ImFont* iconFont_Big;
		static GNS_API ImFont* defaultFont;
		static GNS_API ImFont* boldFont;

		static GNS_API std::vector<gns::gui::GuiWindow*> guiWindows;
		GuiSystem(rendering::Device* device);

		template<typename T, typename... Args>
		static T* RegisterWindow(Args&& ... args)
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
				return newWindow;
			}
			return nullptr;
		}
		template<typename T>
		static T* GetWindow()
		{
			size_t typehash = typeid(T).hash_code();
			for (auto window : guiWindows)
			{
				if(window->typeHash == typehash)
				{
					return dynamic_cast<T*>(window);
				}
			}
			return nullptr;
		};
		void BeginGUI() const;
		void UpdateGui();
		static void PresentGui(const VkCommandBuffer cmd);
		void DisposeGUI() const;
		static GNS_API void ToggleDemoWindow();
	};
}
