#pragma once
#define SDL_MAIN_HANDLED

#ifndef __WIN32__
#define __WIN32__
#endif // __WIN32__
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>

namespace gns
{
	class Window
	{
	public:
		HWND hwndHandle;
		HINSTANCE hinstance;
	private:
		uint32_t WINDOW_WIDTH = 1700;
		uint32_t WINDOW_HEIGHT = 900;

		Sint32 width = 800;
		Sint32 height = 600;

		SDL_Window* sdlWindow;
		SDL_Event sdl_event;
	public:
		Window();
		~Window();

		bool PollEvents();
		void GetExtentions(uint32_t& count, const char** names);
		void GetExtent(int& width, int& height);
		void WindowEvent(const SDL_Event* event);

	private:
		void CreateSDLWindow();
		void DestroyWindow();
	};
}

