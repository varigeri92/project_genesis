#pragma once
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace gns
{
	struct Mesh;
	struct Material;
	struct EntityComponent
	{
		EntityComponent(std::string name) : name{ name } {};
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
		};

		Transform(
			glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) 
			: position{ position }, rotation{rotation }, scale{ scale }
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, glm::vec3(0.f));
			matrix = glm::scale(matrix, glm::vec3(1.f));
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 1.f, 0.f));
		};

		void UpdateMatrix() {
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, position);
			matrix = glm::scale(matrix, scale);
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 0.f, 1.f));
		}
	};

	struct MeshComponent
	{
		std::shared_ptr<Mesh> mesh;
		operator std::shared_ptr<Mesh> () { return mesh; }
		MeshComponent(std::shared_ptr<Mesh> mesh) : mesh{ mesh } {};
	};

	struct MaterialComponent {
		std::shared_ptr<Material> material;
		operator std::shared_ptr<Material>() { return material; }
		MaterialComponent(std::shared_ptr<Material> material) : material{ material } {};
	};
}
