#ifndef ITEM_ENTITY_H
#define ITEM_ENTITY_H

#include "Entity.h"
#include "assetManager/assetHandle.h"
#include "renderer/model.h"

#include <iostream>

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

    [[nodiscard]] nlohmann::json save() const override {
        auto j = Base::save();
        j["itemType"] = static_cast<uint8_t>(_itemType);
        j["consumed"] = _consumed;
        return j;
    }

  protected:
    ItemType _itemType{ItemType::Heal};
    bool _consumed{false};
};
} // namespace game

#endif // ITEM_ENTITY_H
