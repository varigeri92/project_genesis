#pragma once
#include <glm/glm.hpp>
namespace gns::core {
	class CameraSystem;
}

class Camera
{
	friend class gns::core::CameraSystem;

public:
	Camera() = default;
	Camera(float _near, float _far, float fov, float width, float height);

	float _near;
	float _far;
	float fov;

	bool is_perspectivic;
	glm::mat4 GetView() const { return view; }
	glm::mat4 GetProjection() const { return projection; }
private:
	glm::mat4 view;
	glm::mat4 projection;
	float width;
	float height;
};

