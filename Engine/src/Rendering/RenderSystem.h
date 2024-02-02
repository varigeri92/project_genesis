#pragma once
#include "Renderer.h"

namespace gns::rendering
{
	class RenderSystem
	{
	public:
		static gns::rendering::Renderer* S_Renderer;
		static gns::rendering::Device* S_Device;
	};
}
