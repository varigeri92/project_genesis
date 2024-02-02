#pragma once
#include "EnTT/entt.hpp"
#include "GUI/GnsGui.h"
#include "GUI/GuiWindow.h"

namespace gns
{
	struct Entity;
}

namespace gns
{
	struct Scene;
}

using namespace gns::gui;
namespace gns::editor
{
	class EntityInspector : public GuiWindow
	{
		Scene* m_scene;
		entt::entity m_entity;
		bool is_entitySelected = false;
	public:
		EntityInspector(GUI* gui)
			: GuiWindow("Scene Inspector", gui)
		{
		}

		void OnGUI() override;
		void SetInspectedEntity(entt::entity entity, Scene* scene);
	};

	class SceneHierachy : public GuiWindow
	{
	private:
		Scene* m_scene;
		EntityInspector* m_entityInspector;
		bool m_headerOpen = true;
	public:
		SceneHierachy(GUI* gui, Scene* scene, EntityInspector* inspector)
			: GuiWindow("Scene Hierarchy", gui), m_scene(scene), m_entityInspector(inspector) {}
		void OnGUI() override;

	};
}
