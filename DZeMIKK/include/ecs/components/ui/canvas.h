#ifndef DZEMIKK_CANVAS_H
#define DZEMIKK_CANVAS_H

#include "ecs/component.h"

#include <string>

namespace dzemikk {
class Canvas : public Component {
  public:
    using Base = Component;

    Canvas() = default;

    [[nodiscard]] std::string typeName() const override {
        return "Canvas";
    }
};
} // namespace dzemikk

#endif // DZEMIKK_CANVAS_H