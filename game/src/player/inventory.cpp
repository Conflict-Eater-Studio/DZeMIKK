#include "player/inventory.h"

#include "assetManager/assetmanager.h"
#include "ecs/components/ui/horizontalLayout.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/prefabSerializer.h"

namespace game {

void Inventory::setHorizontalLayout(dzemikk::GameObject* horizontalLayout) {
    _horizontalLayout = horizontalLayout;
}

dzemikk::GameObject* Inventory::getHorizontalLayout() const {
    return _horizontalLayout;
}

void Inventory::setMainScene(dzemikk::Scene* mainScene) {
    _mainScene = mainScene;
}

void Inventory::setAssetManager(dzemikk::AssetManager* assetManager) {
    _assetManager = assetManager;
}

void Inventory::addItem(dzemikk::GameObject* item) const {
    if (_horizontalLayout == nullptr || item == nullptr) {
        return;
    }

    _horizontalLayout->addChild(item);

    auto* horizontalLayoutComponent = _horizontalLayout->getComponent<dzemikk::HorizontalLayout>();
    if (horizontalLayoutComponent != nullptr) {
        horizontalLayoutComponent->rebuild();
    }
}

void Inventory::addItem(ItemName itemName) {
    if (_mainScene == nullptr || _assetManager == nullptr) {
        return;
    }

    const auto prefabIt = _itemPrefabs.find(itemName);
    if (prefabIt == _itemPrefabs.end() || prefabIt->second.get() == nullptr) {
        return;
    }

    auto* itemInstance = dzemikk::PrefabSerializer::instantiate(
        *_mainScene, *prefabIt->second.get(), _assetManager);
    addItem(itemInstance);
}

void Inventory::setItem1Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab) {
    _itemPrefabs[Item1] = prefab;
}

void Inventory::setItem2Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab) {
    _itemPrefabs[Item2] = prefab;
}

void Inventory::setItem3Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab) {
    _itemPrefabs[Item3] = prefab;
}

} // namespace game
