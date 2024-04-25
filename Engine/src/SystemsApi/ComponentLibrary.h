#pragma once
#include <memory>
#include <glm/gtx/transform.hpp>
#include "glm/glm.hpp"
#include "../Renderer/Rendering.h"

constexpr float PI = 3.14159265359f;

namespace gns
{
	namespace rendering
	{
		struct Material;
		struct Mesh;
	};

	struct EntityComponent {
		std::string name;
		EntityComponent(std::string entityName) : name(entityName){}
	};

	struct SceneComponent
	{
		size_t runtimeSceneId;
	};

	struct Transform
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		glm::mat4 matrix;

		void UpdateMatrix()
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, position);
			matrix = glm::scale(matrix, scale);
			matrix = glm::rotate(matrix, rotation.x * (PI / 180), glm::vec3(1.f, 0.f, 0.f));
			matrix = glm::rotate(matrix, rotation.y * (PI / 180), glm::vec3(0.f, 1.f, 0.f));
			matrix = glm::rotate(matrix, rotation.z * (PI / 180), glm::vec3(0.f, 0.f, 1.f));
		}
		Transform()
			: position{ 0,0,0 }, rotation{ 0,0,0 }, scale{ 1,1,1 }
		{
			UpdateMatrix();
		}

		Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
			: position{ position }, rotation{ rotation }, scale{ scale }
		{
			UpdateMatrix();
		}

	};
	struct Children
	{
		std::vector<std::shared_ptr<Transform>> children;
	};
	struct Parent
	{
		std::shared_ptr<Transform> parent;
	};

	struct RendererComponent
	{
		std::shared_ptr<rendering::Mesh> mesh;
		std::shared_ptr<rendering::Material> material;
		RendererComponent(std::shared_ptr<rendering::Mesh> mesh, std::shared_ptr<rendering::Material> material)
			: mesh(mesh), material(material)
		{
			LOG_INFO(GREEN << "[Componet Created]:" << DEFAULT << " RendererComponent");
		}
		~RendererComponent()
		{
			LOG_INFO(YELLOW<<"[Componet Removed]:"<<DEFAULT << " RendererComponent");
		};
	};

	struct Camera
	{
		friend class RenderSystem;
		friend class CameraSystem;
		float _near;
		float _far;
		float fov;
		float width;
		float height;

	private:
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 camera_matrix;

	public:
		Camera() = delete;
		Camera(float near, float far, float fov, float width, float height, const Transform& transform) :
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
	};
};
