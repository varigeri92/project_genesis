#pragma once
#include <memory>
#include <glm/gtx/transform.hpp>
#include "glm/glm.hpp"
#include "../Renderer/Rendering.h"
#include "../Utils/Guid.h"
#include "../Utils/Serialization/Serializeable.h"
#include "../Utils/Serialization/Serializer.h"

constexpr float PI = 3.14159265359f;

namespace gns
{
	struct ComponentBase
	{
		virtual void Register() = 0;
	};

	template<typename T, typename... Args>
	void RegisterComponent()
	{
		T* cmp = new T();
		ComponentBase* cmpBase = dynamic_cast<ComponentBase*>(cmp);
		cmpBase->Register();
		delete cmp;
	}

	namespace rendering
	{
		struct Material;
		struct Mesh;
	};

	struct EntityComponent : public ComponentBase {
		std::string name;
		core::guid guid;
		void Register() override
		{
			REGISTER_CMP(EntityComponent);
			REGISTER_FIELD(std::string, name);
			REGISTER_FIELD(core::guid, guid);
		}
		EntityComponent() = default;
		EntityComponent(std::string entityName) : name(entityName), guid(core::Guid::GetNewGuid()){}
	};

	struct SceneComponent
	{
		size_t runtimeSceneId;
	};

	struct Transform : public ComponentBase
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		glm::mat4 matrix;

		void Register() override
		{
			REGISTER_CMP(Transform);
			REGISTER_FIELD(glm::vec3, position);
			REGISTER_FIELD(glm::vec3, rotation);
			REGISTER_FIELD(glm::vec3, scale);
		}

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

	struct RendererComponent : public ComponentBase
	{
		gns::core::guid meshGuid;
		std::shared_ptr<rendering::Mesh> mesh;
		gns::core::guid materialGuid;
		std::shared_ptr<rendering::Material> material;
		RendererComponent() = default;

		RendererComponent(std::shared_ptr<rendering::Mesh> mesh, std::shared_ptr<rendering::Material> material)
			: meshGuid(gns::core::Guid::GetNewGuid()), mesh(mesh),
		materialGuid(gns::core::Guid::GetNewGuid()), material(material)
		{
		}


		void Register() override
		{
			REGISTER_CMP(RendererComponent);
			REGISTER_FIELD(gns::core::guid, meshGuid);
			REGISTER_FIELD(gns::core::guid, materialGuid);
		}
	};

	struct Camera : public ComponentBase
	{
		friend class RenderSystem;
		friend class CameraSystem;
		float _near;
		float _far;
		float fov;
		float width;
		float height;

		void Register() override
		{
			REGISTER_CMP(Camera);
			REGISTER_FIELD(float, _near);
			REGISTER_FIELD(float, _far);
			REGISTER_FIELD(float, fov);
			REGISTER_FIELD(float, width);
			REGISTER_FIELD(float, height);
		}

	private:
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 camera_matrix;

	public:
		Camera() = default;
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
