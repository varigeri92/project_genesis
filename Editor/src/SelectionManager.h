#pragma once
#include "Engine.h"

class SelectionManager
{
public:
	static entt::entity currentSelectedEntity;
	static gns::AssetMetadata* currentSelectedMetadata;
	static bool isSelectionImported;

	static void SetSelectedEntity(entt::entity);
	static void SetSelectedObject(const std::string& name);
	static gns::Event<void, entt::entity> onSelectionChanged;
};

