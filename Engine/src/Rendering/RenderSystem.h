#pragma once
#include "Renderer.h"
#include "../ECS/SystemBase.h"

namespace gns::rendering
{
	class RenderSystem
	{
	public:
		static GEN_API gns::rendering::Renderer* S_Renderer;
		static GEN_API gns::rendering::Device* S_Device;

	};
}
