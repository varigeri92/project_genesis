#pragma once
#include <glm/glm.hpp>
class Camera
{

public:
	glm::vec3 position;
	glm::vec3 rotation;

	float_t near;
	float_t far;
	float_t fov;

	bool is_perspectivic;

	Camera() = default;
	Camera(glm::vec3 pos, glm::vec3 rot, float_t near, float_t far, float_t fov, float_t width, float_t height);
	glm::mat4 GetView() const { return view; }
	glm::mat4 GetProjection() const { return projection; }
private:
	glm::mat4 view;
	glm::mat4 projection;
	float_t width;
	float_t height;
};

