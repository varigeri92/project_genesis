#include "gnspch.h"
#include "MeshData.h"

namespace gns::rendering
{
	MeshData::~MeshData()
	{
		MeshData::Dispose();
	}

	void MeshData::Dispose()
	{
		Device* device = SystemsAPI::GetSystem<RenderSystem>()->GetDevice();
		device->DisposeBuffer(_vertexBuffer);
		device->DisposeBuffer(_indexBuffer);
	}

	Mesh::~Mesh()
	{
		CleanUpMesh();
	};

	void Mesh::Dispose()
	{
		CleanUpMesh();
		Object::Dispose();
	}

	void Mesh::CleanUpMesh()
	{
		for (size_t i = 0; i < m_subMeshes.size(); i++)
		{
			delete m_subMeshes[i];
		}
		m_subMeshes.clear();
	}
}
