#pragma once
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

constexpr float PI = 3.14159265359f;
namespace gns::rendering
{
	struct Mesh;
	struct Material;
}
using namespace gns::rendering;
namespace gns
{
	struct EntityComponent
	{
		EntityComponent(std::string name) : name{ name } {}
		std::string name;
		bool isEnabled = true;
		bool isStatic = false;
	};

	struct Transform
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		glm::mat4 matrix;

		Transform() : position{0,0,0}, rotation { 0,0,0 }, scale { 1,1,1 }
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, glm::vec3(0.f));
			matrix = glm::scale(matrix, glm::vec3(1.f));
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 1.f, 0.f));
		}

		Transform(
			glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) 
			: position{ position }, rotation{rotation }, scale{ scale }
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, glm::vec3(0.f));
			matrix = glm::scale(matrix, glm::vec3(1.f));
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 1.f, 0.f));
		}

		void UpdateMatrix() {
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, position);
			matrix = glm::scale(matrix, scale);
			matrix = glm::rotate(matrix, rotation.x * (PI / 180), glm::vec3(1.f, 0.f, 0.f));
			matrix = glm::rotate(matrix, rotation.y * (PI / 180), glm::vec3(0.f, 1.f, 0.f));
			matrix = glm::rotate(matrix, rotation.z * (PI / 180), glm::vec3(0.f, 0.f, 1.f));
		}
	};

	struct MeshComponent
	{
		std::shared_ptr<Mesh> mesh;
		MeshComponent(std::shared_ptr<Mesh> mesh) : mesh{ mesh } {}
	};

	struct MaterialComponent {
		std::shared_ptr<Material> material;
		MaterialComponent(std::shared_ptr<Material> material) : material{ material } {}
	};

	struct RendererComponent
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		RendererComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : mesh(mesh), material(material) {}
	};

	struct CameraComponent
	{
		CameraComponent() = delete;
		CameraComponent(float near, float far, float fov, float width, float height, Transform& transform) :
			_near(near), _far(far), fov(fov), width(width), height(height)
		{
			view = glm::translate(glm::mat4(1.f), transform.position);
			view = glm::rotate(view, transform.rotation.x, glm::vec3(1.f, 0.f, 0.f));
			view = glm::rotate(view, transform.rotation.y, glm::vec3(0.f, 1.f, 0.f));
			view = glm::rotate(view, transform.rotation.z, glm::vec3(0.f, 0.f, 1.f));
			projection = glm::perspective(glm::radians(fov), (width / height), _near, _far);
			projection[1][1] *= -1;
			camera_matrix = projection * view;
		}

		float _near;
		float _far;
		float fov;
		float width;
		float height;

		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 camera_matrix;
	};
}
