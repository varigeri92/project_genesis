#pragma once
#include "../Core/Components.h"
#include "../Camera.h"

namespace gns::core {
	class CameraSystem
	{
		float m_cameraMoveSpeed = 3.f;
		Transform& transform;
		Camera& camera;
		public:
		CameraSystem(Transform& transform, Camera& camera);
		void UpdateSystem();
		void UpdateProjection(int w, int h);
	};
}

