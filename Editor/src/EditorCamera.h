#pragma once
#include "../../Engine/src/SystemsApi/ComponentLibrary.h"
#include "../../Engine/src/SystemsApi/SystemBase.h"

namespace gns::editor
{
	class Editor;
}

class EditorCamera : public gns::SystemBase
{
	friend class gns::editor::SceneViewWindow;
	friend class gns::editor::Editor;
	;
	gns::Transform m_transform;
	gns::Camera m_camera;
	float m_cameraMoveSpeed;
public:
	EditorCamera();
	void OnCreate() override;
	void OnUpdate(float deltaTime) override;
	void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);
	void UpdateCamera(float deltaTime);
	void UpdateProjection(int w, int h);
};
