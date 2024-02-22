#pragma once
#include "Renderer.h"
#include "../ECS/SystemBase.h"

namespace gns::rendering
{
	class RenderSystem
	{
	public:
		static GNS_API gns::rendering::Renderer* S_Renderer;
		static GNS_API gns::rendering::Device* S_Device;

	};
}
