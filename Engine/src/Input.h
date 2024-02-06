#pragma once
#include <SDL2/SDL_keycode.h>
#include <map>

#include "gnsAPI.h"
#include "glm/glm.hpp"



typedef union SDL_Event;
namespace gns
{
	class Window;
	struct FrameInput
	{
		std::map<int, bool> keysDown;
		std::map<int, bool> keysHeld;
		std::map<int, bool> keysUp;

		std::map<int, bool> mouseDown;
		std::map<int, bool> mouseHeld;
		std::map<int, bool> mouseUp;

	};


	class Input
	{
	public:
		inline static FrameInput frameInput{};

		static GEN_API bool GetKey(int keyCode);
		static GEN_API bool GetKeyUp(int keyCode);
		static GEN_API bool GetKeyDown(int keyCode);
		static GEN_API bool ProcessInput(SDL_Event& event);
		static GEN_API bool GetMouseButtonDown(int mouseButton);
		static GEN_API bool GetMouseButton(int mouseButton);
		static GEN_API bool GetMouseButtonUp(int mouseButton);
		static GEN_API glm::vec2 GetMouseVelocity();

		inline static glm::vec2 mousePos;
		inline static glm::vec2 p_mousePos;
		inline static glm::vec2 mouseVelocity;

		inline static gns::Window* window;
	private:
		void ProcessKeyboard();
		inline static FrameInput previousFrameInput{};


	};
}
