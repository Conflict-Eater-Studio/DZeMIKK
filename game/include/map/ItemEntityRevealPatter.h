#ifndef ITEM_ENTITY_REVEAL_PATTERN_H
#define ITEM_ENTITY_REVEAL_PATTERN_H

#include "ItemEntity.h"

namespace game {
class ItemEntityRevealPattern : public ItemEntity {
  public:
    using Base = ItemEntity;

    ItemEntityRevealPattern() {
        setItemType(ItemType::RevealPattern);
    };

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntityRevealPattern";
    }
};
} // namespace game

#endif // ITEM_ENTITY_REVEAL_PATTERN_H
