#pragma once
#include "genesis.h"

namespace gns {
	class CameraSystem : public gns::core::SystemBase
	{
		float m_cameraMoveSpeed = 3.f;
		Transform& transform;
		CameraComponent& camera;
		public:
		CameraSystem(Transform& transform, CameraComponent& camera);
		~CameraSystem() = default;

		void UpdateCamera();
		void UpdateProjection(int w, int h);

	protected:
		void OnCreate() override;
		void OnUpdate() override;
		void OnDestroy() override;
	private:
		void UpdateViewMatrix();
	public:
		void LogSomething();
	};
}

