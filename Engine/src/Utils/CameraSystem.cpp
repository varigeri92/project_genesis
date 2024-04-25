#include "gnspch.h"
#include "CameraSystem.h"
#include "Time.h"
#include "../Window/InputBackend.h"
#include "../SystemsApi/ComponentLibrary.h"

void gns::CameraSystem::OnCreate()
{
	LOG_INFO("Camera System Created!");
	auto cameraView = SystemsAPI::GetDefaultRegistry().view<Transform, Camera>();

	for (auto [entt, transform, camera] : cameraView.each())
	{
		m_camera = &camera;
		m_transform = &transform;
	}
}

void gns::CameraSystem::OnUpdate(float deltaTime)
{
	PROFILE_FUNC
	UpdateCamera(deltaTime);
}

void gns::CameraSystem::UpdateCamera(float deltaTime)
{

	if (InputBackend::GetMouseButton(3))
	{
		float speed = (deltaTime * m_cameraMoveSpeed);
		float mouse_sensitivity = (deltaTime * 50);
		m_transform->rotation.x += -InputBackend::GetMouseVelocity().y * mouse_sensitivity;
		m_transform->rotation.y += InputBackend::GetMouseVelocity().x * mouse_sensitivity;

		glm::vec3 camFront;
		camFront.x = -cos(glm::radians(m_transform->rotation.x)) * sin(glm::radians(m_transform->rotation.y));
		camFront.y = sin(glm::radians(m_transform->rotation.x));
		camFront.z = cos(glm::radians(m_transform->rotation.x)) * cos(glm::radians(m_transform->rotation.y));
		camFront = glm::normalize(camFront);
		camFront.y *= -1;


		const glm::vec3 camRight = glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::vec3 camUp = glm::cross(-camFront, -camRight);


		//fwd:
		if (InputBackend::GetKey(SDLK_w)) {
			m_transform->position += camFront * speed;
		}
		//back:
		if (InputBackend::GetKey(SDLK_s)) {
			m_transform->position -= camFront * speed;
		}
		//left:
		if (InputBackend::GetKey(SDLK_a)) {
			m_transform->position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, -1.0f, 0.0f))) * speed;
		}
		//right:
		if (InputBackend::GetKey(SDLK_d)) {
			m_transform->position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, -1.0f, 0.0f))) * speed;
		}

		//down:
		if (InputBackend::GetKey(SDLK_q)) {
			m_transform->position -= glm::vec3(0.0f, -1.0f, 0.0f) * speed;
		}
		//up:
		if (InputBackend::GetKey(SDLK_e)) {
			m_transform->position += glm::vec3(0.0f, -1.0f, 0.0f) * speed;
		}


	}

	m_camera->view = glm::translate(glm::mat4(1.f), m_transform->position);
	glm::mat4 rotM = glm::mat4(1.0f);
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_camera->camera_matrix = m_camera->projection * (rotM * m_camera->view);
}


void gns::CameraSystem::UpdateProjection(const int w, const int h)
{
	m_camera->width = w;
	m_camera->height = h;
	m_camera->projection = glm::perspective(
		glm::radians(m_camera->fov), (m_camera->width / m_camera->height), m_camera->_near, m_camera->_far);
	m_camera->projection[1][1] *= -1;
}
