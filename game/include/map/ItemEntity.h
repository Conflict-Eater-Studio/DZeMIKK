#ifndef ITEM_ENTITY_H
#define ITEM_ENTITY_H

#include "Entity.h"
#include "assetManager/assetHandle.h"
#include "renderer/model.h"

#include <iostream>

#include <ecs/gameobject.h>

namespace game {
class ItemEntity : public Entity {
  public:
    using Base = Entity;

    enum class ItemType : uint8_t { Heal, RevealPattern, RevealHex, BonusHex };

    ItemEntity() = default;

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntity";
    }

    void onEnter(HexCellPtr cell) override {
        setCell(cell);

        if (_itemType == ItemType::Heal) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.5F, 0.0F));
        } else if (_itemType == ItemType::BonusHex) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.6F, 0.0F));
        } else if (_itemType == ItemType::RevealHex) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.7F, 0.0F));
        } else if (_itemType == ItemType::RevealPattern) {
            getOwner()->transform()->setPosition(
                cell->getCoord().toWorldPosition(1.0F, 0.1F, cell->getHeight()) +
                glm::vec3(0.0F, 0.5F, 0.0F));
        }

    };
    void onExit() override {};

    void setItemType(ItemType type) {
        _itemType = type;
    }
    
    [[nodiscard]] ItemType getItemType() const {
        return _itemType;
    }

    void consume() {
        _consumed = true;
    }
    [[nodiscard]] bool isConsumed() const {
        return _consumed;
    }

  protected:
    ItemType _itemType{ItemType::Heal};
    bool _consumed{false};
};
} // namespace game

#endif // ITEM_ENTITY_H
