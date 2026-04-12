#ifndef DZEMIKK_UIBUTTON_H
#define DZEMIKK_UIBUTTON_H

#include "ecs/components/ui/iUIInteractable.h"

#include <functional>

namespace dzemikk {
template <typename T> class UIButton : public IUIInteractable {
  public:
    using Base = IUIInteractable;

    [[nodiscard]] std::string typeName() const override {
        return "UIButton";
    }

    T onClick() override {
        if (_onClick) {
            return _onClick();
        }
        return nullptr;
    };
    void onEnter() override;
    void onExit() override;

    void setOnClick(std::function<T()> onClick);

  private:
    std::function<T()> _onClick;
};
} // namespace dzemikk

#endif // DZEMIKK_UIBUTTON_H