#pragma once
#include <EnTT/entt.hpp>
#include <GUI/GuiWindow.h>

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
		EntityInspector()
			: GuiWindow("Inspector") {}

		void OnGUI() override;
		void SetInspectedEntity(entt::entity entity, Scene* scene);
	};

	class SceneHierarchy : public GuiWindow
	{
	private:
		Scene* m_scene;
		EntityInspector* m_entityInspector;
		bool m_headerOpen = true;
	public:
		SceneHierarchy();
		void OnGUI() override;

	};
}
