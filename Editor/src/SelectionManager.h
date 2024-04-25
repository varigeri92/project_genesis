#pragma once
#include "Engine.h"

class SelectionManager
{
public:
	static entt::entity currentSelectedEntity;

	static void SetSelectedEntity(entt::entity);
	static gns::Event<void, entt::entity> onSelectionChanged;
};

