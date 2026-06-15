#pragma once
#ifndef DZEMIKK_INVENTORY_H
#define DZEMIKK_INVENTORY_H

#include "ecs/components/monobehaviour.h"
#include "map/ItemEntity.h"

class Game;

namespace game {
class Inventory : public dzemikk::MonoBehaviour {
  public:
    using Base = MonoBehaviour;

    Inventory() = default;

    [[nodiscard]] std::string typeName() const override {
        return "Inventory";
    }

    void addItem(ItemEntity::ItemType item, unsigned int count = 1);
    void tryUseItem(ItemEntity::ItemType item);
    [[nodiscard]] const std::unordered_map<ItemEntity::ItemType, unsigned int>& getItems() const {
        return _items;
    }
    void setGame(Game* game) {
        _game = game;
    };

  private:
    std::unordered_map<ItemEntity::ItemType, unsigned int> _items;
    Game* _game{nullptr};
};
} // namespace game

#endif
