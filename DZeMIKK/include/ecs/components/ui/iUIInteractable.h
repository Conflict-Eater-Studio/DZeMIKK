#ifndef DZEMIKK_IUIINTERACTABLE_H
#define DZEMIKK_IUIINTERACTABLE_H

#include "ecs/component.h"

namespace dzemikk {
class IUIInteractable : public Component {
  public:
    using Base = Component;

    [[nodiscard]] std::string typeName() const override {
        return "IUIInteractable";
    }

    virtual void onClick() = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
};
} // namespace dzemikk

#endif // DZEMIKK_IUIINTERACTABLE_H