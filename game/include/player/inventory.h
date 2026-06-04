#pragma once
#ifndef DZEMIKK_INVENTORY_H
#define DZEMIKK_INVENTORY_H

#include "assetManager/assetHandle.h"
#include "ecs/components/monobehaviour.h"
#include "ecs/serialize/uuid.h"

namespace dzemikk {
class GameObject;
class Scene;
class AssetManager;
}

namespace game {
class Inventory : public dzemikk::MonoBehaviour {
public:
    using Base = MonoBehaviour;

    enum ItemName {
        Item1,
        Item2,
        Item3,
    };

    void setHorizontalLayout(dzemikk::GameObject* horizontalLayout);
    [[nodiscard]] dzemikk::GameObject* getHorizontalLayout() const;

    void setMainScene(dzemikk::Scene* mainScene);
    void setAssetManager(dzemikk::AssetManager* assetManager);

    void addItem(ItemName itemName);

    [[nodiscard]] std::string typeName() const override {
        return "Inventory";
    }

    void setItem1Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab);
    void setItem2Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab);
    void setItem3Prefab(const dzemikk::AssetHandle<nlohmann::json>& prefab);

private:
    void addItem(dzemikk::GameObject* item) const;
    dzemikk::GameObject* _horizontalLayout = nullptr;
    dzemikk::Scene* _mainScene = nullptr;
    dzemikk::AssetManager* _assetManager = nullptr;
    std::unordered_map<ItemName, dzemikk::AssetHandle<nlohmann::json>> _itemPrefabs;
};
}

#endif
