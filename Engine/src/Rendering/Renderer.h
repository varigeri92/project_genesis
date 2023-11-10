#pragma once
#include "rendererfwd.h"
#include <vector>

namespace gns::rendering
{
	class Renderer
	{
		typedef struct FrameData{
			
		} FrameData;

	public:
		Renderer(Window* window);
		~Renderer();
		Renderer operator=(Renderer& other) = delete;

	private:
		Device* m_device;
		uint32_t m_framesInFlight;
		std::vector<FrameData> m_frames;
	};
}
