#pragma once
#include "../SystemsApi/SystemBase.h"
#include "glm/glm.hpp"

namespace gns
{
	struct Camera;
	struct Transform;

	class CameraSystem : public SystemBase
{
	private:
		Camera* m_camera;
		Transform* m_transform;
		float m_cameraMoveSpeed = 3.f;
		bool flipY = true;
		void UpdateCamera(float deltaTime);
	public:
		GNS_API CameraSystem() = default;
		CameraSystem(CameraSystem& other) = delete;
		GNS_API void OnCreate() override;
		GNS_API void OnUpdate(float deltaTime) override;
		GNS_API void UpdateProjection(const int w, const int h);
		GNS_API void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
};
}
