#include "CameraSystem.h"
#include "genesis.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL2/SDL_keycode.h"

bool flipY = true;
namespace gns
{
	gns::CameraSystem::CameraSystem(Transform& transform, CameraComponent& camera) :
	SystemBase("CameraSystem"), transform{ transform }, camera{ camera } {}


	void gns::CameraSystem::UpdateViewMatrix()
	{
	}
	void CameraSystem::LogSomething()
	{
		LOG_INFO("HELLO CMAERA!!!");
	}


	void gns::CameraSystem::UpdateCamera()
	{

		float speed = (Time::GetDelta() * m_cameraMoveSpeed);
		if(Input::GetMouseButton(3))
		{
			Window::SetMouseRelative(true);

			float mouse_sensitivity = (Time::GetDelta() * 50);
			transform.rotation.x += -Input::GetMouseVelocity().y * mouse_sensitivity;
			transform.rotation.y += Input::GetMouseVelocity().x * mouse_sensitivity;

			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(transform.rotation.x)) * sin(glm::radians(transform.rotation.y));
			camFront.y = sin(glm::radians(transform.rotation.x));
			camFront.z = cos(glm::radians(transform.rotation.x)) * cos(glm::radians(transform.rotation.y));
			camFront = glm::normalize(camFront);
			camFront.y *= -1;


			const glm::vec3 camRight = glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f));
			const glm::vec3 camUp = glm::cross(-camFront, -camRight);


			//fwd:
			if (Input::GetKey(SDLK_w)) {
				transform.position += camFront * speed;
			}
			//back:
			if (Input::GetKey(SDLK_s)) {
				transform.position -= camFront * speed;
			}
			//left:
			if (Input::GetKey(SDLK_a)) {
				transform.position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, -1.0f, 0.0f))) * speed;
			}
			//right:
			if (Input::GetKey(SDLK_d)) {
				transform.position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, -1.0f, 0.0f))) * speed;
			}

			//down:
			if (Input::GetKey(SDLK_q)) {
				transform.position -= glm::vec3(0.0f, -1.0f, 0.0f) * speed;
			}
			//up:
			if (Input::GetKey(SDLK_e)) {
				transform.position += glm::vec3(0.0f, -1.0f, 0.0f)* speed;
			}
		}
		else
		{
			Window::SetMouseRelative(false);
		}

		camera.view = glm::translate(glm::mat4(1.f), transform.position);
		glm::mat4 rotM = glm::mat4(1.0f);
		rotM = glm::rotate(rotM, glm::radians(transform.rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		camera.camera_matrix = camera.projection * (rotM * camera.view);
	}

	void gns::CameraSystem::UpdateProjection(int w, int h)
	{
		camera.width = w;
		camera.height = h;
		camera.projection = glm::perspective(glm::radians(camera.fov), (camera.width / camera.height), camera._near, camera._far);
		camera.projection[1][1] *= -1;
	}

	void CameraSystem::OnCreate()
	{
		LOG_INFO("System: " << name << "has ben created!");
	}

	void CameraSystem::OnUpdate()
	{
		UpdateCamera();
	}

	void CameraSystem::OnDestroy()
	{
		LOG_INFO("System: " << name << "has ben Destroyed!");
	}
}
