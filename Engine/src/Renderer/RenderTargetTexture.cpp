#include "gnspch.h"
#include "RenderTargetTexture.h"

#include "RenderSystem.h"

namespace gns::rendering
{
	void RenderTargetTexture::CreateRenderTarget()
	{

		Device* _device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		VkDevice device = _device->m_device;
		
	}
}
