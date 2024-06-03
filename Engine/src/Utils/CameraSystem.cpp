#include "gnspch.h"
#include "CameraSystem.h"
#include "Time.h"
#include "../Window/InputBackend.h"
#include "../SystemsApi/ComponentLibrary.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void gns::CameraSystem::OnCreate()
{
	LOG_INFO("Camera System Created!");
	auto cameraView = SystemsAPI::GetRegistry().view<Transform, Camera>();

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


void gns::CameraSystem::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	glm::mat4 view = glm::mat4{ 1.f };
	view[0][0] = u.x;
	view[1][0] = u.y;
	view[2][0] = u.z;
	view[0][1] = v.x;
	view[1][1] = v.y;
	view[2][1] = v.z;
	view[0][2] = w.x;
	view[1][2] = w.y;
	view[2][2] = w.z;
	view[3][0] = -glm::dot(u, position);
	view[3][1] = -glm::dot(v, position);
	view[3][2] = -glm::dot(w, position);
	m_camera->view = view;
}


void gns::CameraSystem::UpdateCamera(float deltaTime)
{
	if (InputBackend::GetMouseButton(3))
	{
		float speed = (deltaTime * m_cameraMoveSpeed);
		float mouse_sensitivity = (deltaTime * 1.f);
		m_transform->rotation.x += -InputBackend::GetMouseVelocity().y * mouse_sensitivity;
		m_transform->rotation.y += -InputBackend::GetMouseVelocity().x * mouse_sensitivity;

		glm::quat rotation = { m_transform->rotation };
		float pitch = (m_transform->rotation.x); // Rotation around the X-axis
		float yaw = (m_transform->rotation.y);   // Rotation around the Y-axis
		float roll = (m_transform->rotation.z); // Rotation around the Z-axis

		glm::vec3 camFront;
		camFront.x = cos(pitch) * sin(yaw);
		camFront.y = sin(pitch);
		camFront.z = cos(pitch) * cos(yaw);
		camFront = glm::normalize(camFront);
		camFront.y *= -1;
		camFront *= -1;

		glm::vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 camRight = -glm::normalize(glm::cross(worldUp, camFront));
		glm::vec3 camUp = glm::normalize(glm::cross(camFront, camRight));
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
			m_transform->position += camRight * speed;
		}
		//right:
		if (InputBackend::GetKey(SDLK_d)) {
			m_transform->position -= camRight * speed;
		}

		//down:
		if (InputBackend::GetKey(SDLK_q)) {
			m_transform->position += camUp * speed;
		}
		//up:
		if (InputBackend::GetKey(SDLK_e)) {
			m_transform->position -= camUp * speed;
		}


	}
	//m_camera->view = m_transform->matrix;
	m_camera->projection = glm::perspective(
		glm::radians(m_camera->fov), (m_camera->width / m_camera->height), m_camera->_near, m_camera->_far);
	m_camera->projection[1][1] *= -1;
	setViewYXZ(m_transform->position, m_transform->rotation);
	/*
	m_camera->view = glm::translate(glm::mat4(1.f), m_transform->position);
	glm::mat4 rotM = glm::mat4(1.0f);
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	*/
	m_camera->camera_matrix = m_camera->projection * (m_camera->view);
}


void gns::CameraSystem::UpdateProjection(const int w, const int h)
{
	m_camera->width = w;
	m_camera->height = h;
	m_camera->projection = glm::perspective(
		glm::radians(m_camera->fov), (m_camera->width / m_camera->height), m_camera->_near, m_camera->_far);
	m_camera->projection[1][1] *= -1;
}
