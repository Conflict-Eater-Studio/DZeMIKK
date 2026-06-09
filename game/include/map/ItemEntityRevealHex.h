#ifndef ITEM_ENTITY_REVEAL_HEX_H
#define ITEM_ENTITY_REVEAL_HEX_H

#include "ItemEntity.h"

namespace game {
class ItemEntityRevealHex : public ItemEntity {
  public:
    using Base = ItemEntity;

    ItemEntityRevealHex() {
        setItemType(ItemType::RevealHex);
    };

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntityRevealHex";
    }
};
} // namespace game

#endif // ITEM_ENTITY_REVEAL_HEX_H
