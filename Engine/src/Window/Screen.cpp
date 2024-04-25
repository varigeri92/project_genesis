#include "gnspch.h"
#include "Screen.h"

namespace gns
{
	uint32_t Screen::m_width = 0;
	uint32_t Screen::m_height = 0;
	float Screen::m_aspectRatio = 0;
	Screen::ScreenMode Screen::m_screenMode = Screen::ScreenMode::sm_none;

	void Screen::SetResolution(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
		ReCalculateAspectRatio();
	}

	void Screen::SetScreenMode(ScreenMode screenMode)
	{
		m_screenMode = screenMode;
	}

	void Screen::GetResolution(uint32_t& width, uint32_t& height)
	{
		width = m_width;
		height = m_height;
	}

	float Screen::GetAspectRatio()
	{
		return m_aspectRatio;
	}

	void Screen::InitDefaultScreen(uint32_t width, uint32_t height, ScreenMode screenMode)
	{
		m_screenMode = screenMode;
		SetResolution(width, height);
	}

	void Screen::GetDrawSurface()
	{
	}

	void Screen::ReCalculateAspectRatio()
	{
		m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
	}
}
