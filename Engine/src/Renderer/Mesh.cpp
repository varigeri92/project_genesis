#include "gnspch.h"
#include "Mesh.h"

namespace gns::rendering
{
	Mesh::~Mesh()
	{
		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		device->DisposeBuffer(_vertexBuffer);
		device->DisposeBuffer(_indexBuffer);
	}

	void Mesh::Dispose()
	{
		Object::Dispose();
	}
}
