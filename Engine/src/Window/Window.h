#pragma once
#define SDL_MAIN_HANDLED

#ifndef __WIN32__
#define __WIN32__
#endif // __WIN32__
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>


namespace gns
{
	namespace gui
	{
		class GuiSystem;
	}

	class Window
	{
		friend class gns::gui::GuiSystem;
		SDL_Window* sdlWindow;
		SDL_Event sdl_event;
	public:
		GNS_API static Window* getInstance()
		{
			if (instance == nullptr)
				instance = new Window();

			return instance;
		}
		~Window();
		Window(Window const&) = delete;
		void operator=(Window const&) = delete;
	private:
		Window();
		GNS_API static Window* instance;

	public:
		HWND hwndHandle;
		HINSTANCE hinstance;
		bool isMinimized;

		bool PollEvents();
		void GetExtentions(uint32_t& count, const char** names);
		void GetExtent(int& width, int& height);
		void WindowEvent(const SDL_Event* event);
		void InitWindow(uint32_t width, uint32_t height);
	};
}
