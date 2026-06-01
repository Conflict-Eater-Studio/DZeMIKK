#pragma once
#ifndef DZEMIKK_INVENTORY_H
#define DZEMIKK_INVENTORY_H

#include "ecs/components/monobehaviour.h"

namespace dzemikk {
class GameObject;
}

namespace game {
class Inventory final : public dzemikk::MonoBehaviour {
public:
    void setHorizontalLayout(dzemikk::GameObject* horizontalLayout);
    [[nodiscard]] dzemikk::GameObject* getHorizontalLayout() const;

    void addItem(dzemikk::GameObject* item);

    [[nodiscard]] std::string typeName() const override {
        return "Inventory";
    }

private:
    dzemikk::GameObject* _horizontalLayout = nullptr;
};
}

#endif
