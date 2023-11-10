#include "Renderer.h"

#include "Log.h"
#include "rendererInclude.h"

namespace gns::rendering
{
Renderer::Renderer(Window* window) 
{
	m_device = new Device(window);
	m_framesInFlight = m_device->m_imageCount;
	m_frames.resize(m_framesInFlight);
	LOG_INFO(m_framesInFlight);
}

Renderer::~Renderer()
{
	delete(m_device);
}

}