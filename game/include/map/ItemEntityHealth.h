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

    [[nodiscard]] nlohmann::json save() const override {
        auto j = Base::save();
        j["healAmount"] = _healAmount;
        return j;
    }

  private:
    float _healAmount{100.0F};
};
} // namespace game

#endif // ITEM_ENTITY_HEALTH_H
