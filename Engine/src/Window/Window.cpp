#include "Window.h"
#include "vklog.h"
#include "../Input.h"
#include "../Rendering/Renderer.h"


gns::Window::Window()
{
	CreateSDLWindow();
}

gns::Window::~Window()
{
	DestroyWindow();
}

void gns::Window::CreateSDLWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
	}

	sdlWindow = SDL_CreateWindow(
		"",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!sdlWindow) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
	}

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
	hwndHandle = wmInfo.info.win.window;
	hinstance = wmInfo.info.win.hinstance;

	int w = 0;
	int h = 0;
	SDL_Vulkan_GetDrawableSize(sdlWindow, &w, &h);

}

void gns::Window::DestroyWindow()
{
	SDL_DestroyWindow(sdlWindow);
}

bool gns::Window::PollEvents()
{
	/*
	
	while (SDL_PollEvent(&sdl_event)) {
		if (sdl_event.type == SDL_QUIT) {
			return true;
		}
		if(sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			gns::rendering::Renderer::m_frameBufferResized = true;
		}
		if(sdl_event.window.event == SDL_WINDOWEVENT_MINIMIZED)
		{
			gns::rendering::Renderer::m_frameBufferMinimized = true;
		}
		if (sdl_event.window.event == SDL_WINDOWEVENT_RESTORED)
		{
			gns::rendering::Renderer::m_frameBufferMinimized = false;
		}
	}
	return false;
	*/
	return Input::ProcessInput(sdl_event);
}

void gns::Window::GetExtentions(uint32_t& count, const char** names)
{
	if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &count, nullptr))
	{
		LOG_VK_ERROR("failed to get Extensions Count");
	}

	if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &count, names))
	{
		LOG_VK_ERROR("failed to get Extensions names");
	}
}

void gns::Window::GetExtent(int& w, int& h)
{
	SDL_Vulkan_GetDrawableSize(sdlWindow, &w, &h);
}

void gns::Window::WindowEvent(const SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT) {
		switch (event->window.event) {
		case SDL_WINDOWEVENT_SHOWN:
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			break;
		case SDL_WINDOWEVENT_MOVED:
			break;
		case SDL_WINDOWEVENT_RESIZED:
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			break;
		case SDL_WINDOWEVENT_RESTORED:
			break;
		case SDL_WINDOWEVENT_ENTER:
			break;
		case SDL_WINDOWEVENT_LEAVE:
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			break;
		case SDL_WINDOWEVENT_CLOSE:
			break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
		case SDL_WINDOWEVENT_TAKE_FOCUS:
			break;
		case SDL_WINDOWEVENT_HIT_TEST:
			break;
#endif
		default:

			break;
		}
	}
}
