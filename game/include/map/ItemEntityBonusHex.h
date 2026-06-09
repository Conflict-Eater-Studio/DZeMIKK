#ifndef ITEM_ENTITY_BONUS_HEX_H
#define ITEM_ENTITY_BONUS_HEX_H

#include "ItemEntity.h"
#include "map/HexPattern.h"

namespace game {
class ItemEntityBonusHex : public ItemEntity {
  public:
    using Base = ItemEntity;

    ItemEntityBonusHex() : _pattern(HexPattern({{0, 0}}, HexPattern::Type::BONUSHEX)) {
        setItemType(ItemType::BonusHex);
    };
    ItemEntityBonusHex(HexPattern pattern) : _pattern(std::move(pattern)) {
        setItemType(ItemType::BonusHex);
    }

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntityHealth";
    }

    void setHexPattern(HexPattern pattern) {
        _pattern = std::move(pattern);
    }
    [[nodiscard]] HexPattern getHexPattern() const {
        return _pattern;
    }

  private:
    HexPattern _pattern;
};
} // namespace game

#endif // ITEM_ENTITY_HEALTH_H
