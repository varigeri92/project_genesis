#pragma once
#include "../Core/Components.h"

namespace gns {
	class CameraSystem
	{
		float m_cameraMoveSpeed = 3.f;
		Transform& transform;
		CameraComponent& camera;
		public:
		CameraSystem(Transform& transform, CameraComponent& camera);
		void UpdateCamera();
		void UpdateProjection(int w, int h);
	};
}

