#include "player/inventory.h"

#include "ecs/components/ui/horizontalLayout.h"
#include "ecs/gameobject.h"

namespace game {

void Inventory::setHorizontalLayout(dzemikk::GameObject* horizontalLayout) {
    _horizontalLayout = horizontalLayout;
}

dzemikk::GameObject* Inventory::getHorizontalLayout() const {
    return _horizontalLayout;
}

void Inventory::addItem(dzemikk::GameObject* item) {
    if (_horizontalLayout == nullptr || item == nullptr) {
        return;
    }

    _horizontalLayout->addChild(item);

    auto* horizontalLayoutComponent = _horizontalLayout->getComponent<dzemikk::HorizontalLayout>();
    if (horizontalLayoutComponent != nullptr) {
        horizontalLayoutComponent->rebuild();
    }
}

} // namespace game
