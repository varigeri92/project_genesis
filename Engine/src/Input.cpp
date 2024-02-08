#include <SDL2/SDL.h>
#include "Input.h"
#include <string>
#include "Log.h"
#include "Window/Window.h"
#include "ImGui/imgui_impl_sdl2.h"


namespace gns
{
	bool Input::GetKey(int keyCode)
	{
		return frameInput.keysHeld[keyCode];
	}
	bool Input::GetKeyUp(int keyCode)
	{
		return frameInput.keysUp[keyCode];
	}

	bool Input::GetKeyDown(int keyCode)
	{
		return frameInput.keysDown[keyCode];
	}

	bool Input::ProcessInput(SDL_Event& event)
	{
		frameInput.keysDown.clear();
		frameInput.keysUp.clear();

		frameInput.mouseDown.clear();
		frameInput.mouseUp.clear();

		p_mousePos.x = mousePos.x;
		p_mousePos.y = mousePos.y;
		mouseVelocity = { 0,0 };

		/*
		if (mousePos.x < 0) {
			SDL_WarpMouseInWindow(NULL, 1920, mousePos.y);
			p_mousePos.x = 1920;
		}
		if (mousePos.x > 1921) {
			SDL_WarpMouseInWindow(NULL, 0, mousePos.y);
			p_mousePos.x = 0;
		}
		if (mousePos.y < 0) {
			SDL_WarpMouseInWindow(NULL, mousePos.x, 1080 );
			p_mousePos.y = 1080;
		}
		if (mousePos.y > 1081) {
			SDL_WarpMouseInWindow(NULL, mousePos.x, 0);
			p_mousePos.y = 0;
		}
		*/
		while (SDL_PollEvent(&event) != 0)
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			window->WindowEvent(&event);
			switch (event.type)
			{
			case SDL_QUIT:
				return true;
			case SDL_KEYDOWN:
				if (!frameInput.keysDown[event.key.keysym.sym])
				{
					frameInput.keysDown[event.key.keysym.sym] = true;
				}
				frameInput.keysHeld[event.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				frameInput.keysUp[event.key.keysym.sym] = true;
				frameInput.keysHeld[event.key.keysym.sym] = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				frameInput.mouseDown[event.button.button] = true;
				frameInput.mouseHeld[event.button.button] = true;
				break;
			case SDL_MOUSEBUTTONUP:
				frameInput.mouseUp[event.button.button] = true;
				frameInput.mouseHeld[event.button.button] = false;
				break;
			case SDL_MOUSEMOTION:
				mousePos.x = event.motion.x;
				mousePos.y = event.motion.y;
				mouseVelocity.x = event.motion.xrel;
				mouseVelocity.y = event.motion.yrel;
				break;
			default:
				break;
			}

		}
		//mouseVelocity = p_mousePos - mousePos;
		//mouseVelocity.x = (float)mouseVelocity.x / 100.f; // 
		//mouseVelocity.y = (float)mouseVelocity.y / 100.f;
		//mouseVelocity = glm::normalize(mouseVelocity);
		return false;
	}
	bool Input::GetMouseButtonDown(int mouseButton)
	{
		return frameInput.mouseDown[mouseButton];
	}
	bool Input::GetMouseButtonUp(int mouseButton)
	{
		return frameInput.mouseUp[mouseButton];
	}

	glm::vec2 Input::GetMouseVelocity()
	{
		return mouseVelocity;
	}

	bool Input::GetMouseButton(int mouseButton)
	{
		return frameInput.mouseHeld[mouseButton];
	}
}