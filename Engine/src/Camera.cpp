#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float_t _near, float_t _far, float_t fov, float_t width, float_t height):
	_near(_near),
	_far(_far),
	fov(fov),
	width(width),
	height(height)
{
	view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	projection = glm::perspective(glm::radians(fov), (width / height), _near, _far);
	projection[1][1] *= -1;
}
