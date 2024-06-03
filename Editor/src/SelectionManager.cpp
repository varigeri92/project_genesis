#include "SelectionManager.h"

#include "AssetManager/AssetImporter.h"

entt::entity SelectionManager::currentSelectedEntity = entt::null;
gns::AssetMetadata* SelectionManager::currentSelectedMetadata = nullptr;
bool SelectionManager::isSelectionImported = false;

gns::Event<void, entt::entity> SelectionManager::onSelectionChanged = {};
void SelectionManager::SetSelectedEntity(entt::entity entt)
{
	currentSelectedEntity = entt;
	onSelectionChanged.Dispatch(currentSelectedEntity);
}

void SelectionManager::SetSelectedObject(const std::string& name)
{
	currentSelectedEntity = entt::null;
	gns::core::guid assetGuid = 0;
	if(!gns::AssetDatabase::IsAssetImported(name, assetGuid))
		isSelectionImported = false;
	else
		currentSelectedMetadata = &gns::AssetDatabase::GetAssetByGuid(assetGuid);

	onSelectionChanged.Dispatch(currentSelectedEntity);
}

