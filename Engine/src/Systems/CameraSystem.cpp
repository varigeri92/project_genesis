#include "CameraSystem.h"
#include "../Input.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL2/SDL_keycode.h"
#include "../Core/Time.h"

gns::core::CameraSystem::CameraSystem(Transform& transform, Camera& camera) : transform{ transform }, camera {camera}  {}

void gns::core::CameraSystem::UpdateSystem()
{
	float x_velocity = 0.f;
	float y_velocity = 0.f;
	float z_velocity = 0.f;

	//fwd:
	if (Input::GetKey(SDLK_w)) {
		y_velocity = 1.f;
	}
	//back:
	if (Input::GetKey(SDLK_s)) {
		y_velocity = -1.f;
	}
	
	//left:
	if (Input::GetKey(SDLK_a)) {
		x_velocity = -1.f;
	}
	//right:
	if (Input::GetKey(SDLK_d)) {
		x_velocity = 1.f;
	}

	//up:
	if (Input::GetKey(SDLK_q)) {
		z_velocity = -1.f;
	}
	//down:
	if (Input::GetKey(SDLK_e)) {
		z_velocity = 1.f;
	}

	transform.position += glm::vec3(x_velocity, y_velocity, z_velocity) * (Time::GetDelta() * m_cameraMoveSpeed);
	camera.view = glm::lookAt(transform.position, 
		(transform.position + glm::vec3(0.f,1.f,0.f)),
		glm::vec3(0.0f, 0.0f, 1.0f));
}

void gns::core::CameraSystem::UpdateProjection(int w, int h)
{
	camera.width = w;
	camera.height = h;
	camera.projection = glm::perspective(glm::radians(camera.fov), (camera.width / camera.height), camera._near, camera._far);
	camera.projection[1][1] *= -1;
}
