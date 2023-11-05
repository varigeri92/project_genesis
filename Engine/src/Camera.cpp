#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 pos, glm::vec3 rot, float_t near, float_t far, float_t fov, float_t width, float_t height):
	position (pos),
	rotation(rot),
	near(near),
	far(far),
	fov(fov),
	width(width),
	height(height)
{
	view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective(glm::radians(45.0f),
		width / height, 0.1f, 100.0f);
	projection[1][1] *= -1;
}
