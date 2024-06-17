#pragma once
#include <memory>
#include <glm/gtx/transform.hpp>

#include "../AssetDatabase/AssetDatabase.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#include "../Renderer/Rendering.h"
#include "../Utils/Guid.h"
#include "../Utils/Serialization/Serializeable.h"
#include "../Utils/Serialization/Serializer.h"

constexpr float PI = 3.14159265359f;

namespace gns
{
	namespace editor
	{
		class SceneViewWindow;
	}

	struct ComponentBase
	{
		virtual ~ComponentBase() = default;
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
		struct MeshData;
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

		entt::entity parent;
		core::guid parent_guid;

		std::vector<entt::entity> children;
		std::vector<core::guid> children_guid;

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
			glm::quat quaternion = glm::quat(rotation);
			matrix *= glm::toMat4(quaternion);
		}
		Transform()
			: position{ 0,0,0 }, rotation{ 0,0,0 }, scale{ 1,1,1 }
		{
			parent = entt::null;
			UpdateMatrix();
		}

		Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
			: position{ position }, rotation{ rotation }, scale{ scale }
		{
			parent = entt::null;
			UpdateMatrix();
		}
	};

	struct RendererComponent : public ComponentBase
	{
		rendering::Mesh* m_mesh;
		std::vector<rendering::Material*> m_materials;

		RendererComponent() = default;

		RendererComponent(gns::rendering::Mesh* mesh, rendering::Material* material): m_mesh(mesh), m_materials()
		{
			m_materials.push_back(material);
			m_materials.push_back(material);
		}

		RendererComponent(core::guid meshReferenceGuid, core::guid materialReferenceGuid): m_materials{}
		{
			AssignMesh(meshReferenceGuid);
			AssignMaterial(materialReferenceGuid);
		}

		void AssignMesh(core::guid meshReferenceGuid)
		{
			AssetMetadata meshMeta = AssetDatabase::GetAssetByGuid(meshReferenceGuid);
			m_mesh = Object::Get<rendering::Mesh>(meshMeta.guid);
		}
		void AssignMaterial(core::guid materialReferenceGuid)
		{
			rendering::Material* material = nullptr;
			if(materialReferenceGuid == 0)
			{
				AssetMetadata default_materialMeta = AssetDatabase::GetAssetByName("__default_material");
				material = Object::Get<rendering::Material>(default_materialMeta.guid);
				for(const auto& subMesh : m_mesh->m_subMeshes)
				{
					m_materials.push_back(material);
				}
				return;
			}
			AssetMetadata materialMeta = AssetDatabase::GetAssetByGuid(materialReferenceGuid);
			material = Object::Get<rendering::Material>(materialMeta.guid);
			for (const auto& subMesh : m_mesh->m_subMeshes)
			{
				m_materials.push_back(material);
			}
			
		}

		void Register() override
		{
			REGISTER_CMP(RendererComponent);
			REGISTER_FIELD(rendering::Mesh, m_mesh);
			REGISTER_FIELD(std::vector<rendering::Material*>, m_materials);
		}

	};

	struct Camera : public ComponentBase
	{
		friend class gns::editor::SceneViewWindow;
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
