#ifndef ITEM_ENTITY_HEALTH_H
#define ITEM_ENTITY_HEALTH_H

#include "ItemEntity.h"

namespace game {
class ItemEntityHealth : public ItemEntity {
  public:
    using Base = ItemEntity;

    ItemEntityHealth() {
        setItemType(ItemType::Heal);
    };
    ItemEntityHealth(float healAmount) : _healAmount(healAmount) {
        setItemType(ItemType::Heal);
    }

    [[nodiscard]] std::string typeName() const override {
        return "ItemEntityHealth";
    }

    void setHealAmount(float amount) {
        _healAmount = amount;
    }
    [[nodiscard]] float getHealAmount() const {
        return _healAmount;
    }

  private:
    float _healAmount{10.0F};
};
} // namespace game

#endif // ITEM_ENTITY_HEALTH_H
