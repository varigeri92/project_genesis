#pragma once
#include "Engine.h"
namespace gns
{
	class Screen
	{
		friend class Engine;

		static uint32_t m_width;
		static uint32_t m_height;
		static float m_aspectRatio;

	public:
		static enum class ScreenMode
		{
			sm_none = 0,
			sm_fullscreen = 1,
			sm_windowed = 2,
			sm_borderless = 4,
		} m_screenMode;


		GNS_API static void SetResolution(uint32_t width, uint32_t height);
		GNS_API static void SetScreenMode(ScreenMode screenMode);

		GNS_API static void GetResolution(uint32_t& width, uint32_t& height);
		GNS_API static float GetAspectRatio();

	private:
		static void InitDefaultScreen(uint32_t width, uint32_t height, ScreenMode screenMode);
		static void GetDrawSurface();
		static void ReCalculateAspectRatio();
	};
}
