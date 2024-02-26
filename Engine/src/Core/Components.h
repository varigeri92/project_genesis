#pragma once
#include <iostream>
#include <typeinfo>

#include "Log.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../AssetDatabase/Guid.h"
#include "Serializeable.h"

#define EDITOR_BUILD
using namespace gns::rendering;

constexpr float PI = 3.14159265359f;
namespace gns::rendering
{
	struct Mesh;
	struct Material;
}

struct SerializedProperty
{
	std::string type = 0;
	std::string  name = 0;
	void* data = 0;
};

#ifdef EDITOR_BUILD
#define SERIALIZE(type, name, value)type name = value;\
	SerializedProperty serialized_##name = {#type, #name, &name}

#define REGISTER(name) serializedProperties.push_back(&serialized_##name)

#else
#define SERIALIZE(type, name, value) type name = value
#endif

#define COMPONENT(name) struct name : public gns::ComponentBase


namespace gns
{
	struct ComponentBase
	{
		core::guid guid;
		std::vector<SerializedProperty*> serializedProperties = {};
		ComponentBase() : guid(core::Guid::GetNewGuid()){}
	};

	COMPONENT(EntityComponent)
	{
		SERIALIZE(std::string, name, "");
		SERIALIZE(bool, isEnabled, true);
		SERIALIZE(bool, isStatic, true);

		EntityComponent(std::string name) : name{ name }
		{
			REGISTER(name);
			REGISTER(isEnabled);
			REGISTER(isStatic);
			LOG_INFO("FASZ KIVAN " << name);
			if (std::empty(name)) name = "New Entity";
		}
	};

	COMPONENT(Transform)
	{
		SERIALIZE(glm::vec3, position, {});
		SERIALIZE(glm::vec3, rotation, {});
		SERIALIZE(glm::vec3, scale, {});

		glm::mat4 matrix;

		void Register()
		{
			REGISTER(position);
			REGISTER(rotation);
			REGISTER(scale);
		}

		Transform() : position{0,0,0}, rotation { 0,0,0 }, scale { 1,1,1 }
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, glm::vec3(0.f));
			matrix = glm::scale(matrix, glm::vec3(1.f));
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 1.f, 0.f));

			Register();
		}

		Transform(
			glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) 
			: position{ position }, rotation{rotation }, scale{ scale }
		{
			matrix = glm::mat4(1.f);
			matrix = glm::translate(matrix, glm::vec3(0.f));
			matrix = glm::scale(matrix, glm::vec3(1.f));
			matrix = glm::rotate(matrix, 0.f, glm::vec3(0.f, 1.f, 0.f));

			Register();
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

	COMPONENT(MeshComponent)
	{
		std::shared_ptr<Mesh> mesh;
		MeshComponent(std::shared_ptr<Mesh> mesh) : mesh{ mesh } {}
	};

	COMPONENT(MaterialComponent) {
		std::shared_ptr<Material> material;
		MaterialComponent(std::shared_ptr<Material> material) : material{ material } {}
	};

	COMPONENT(RendererComponent)
	{
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<Material> material;
		RendererComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : mesh(mesh), material(material) {}
	};

	COMPONENT(CameraComponent)
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
