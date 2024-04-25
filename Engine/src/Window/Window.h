﻿#pragma once
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
		Window(uint32_t width, uint32_t height);
		~Window();
		HWND hwndHandle;
		HINSTANCE hinstance;


		bool PollEvents();
		void GetExtentions(uint32_t& count, const char** names);
		void GetExtent(int& width, int& height);
		void WindowEvent(const SDL_Event* event);
	};
}
