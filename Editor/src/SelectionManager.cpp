#include "SelectionManager.h"

entt::entity SelectionManager::currentSelectedEntity = entt::null;
gns::Event<void, entt::entity> SelectionManager::onSelectionChanged = {};
void SelectionManager::SetSelectedEntity(entt::entity entt)
{
	currentSelectedEntity = entt;
	onSelectionChanged.Dispatch(entt);
}
