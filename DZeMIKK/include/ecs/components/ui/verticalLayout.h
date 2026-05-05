#ifndef DZEMIKK_VERTICALLAYOUT_H
#define DZEMIKK_VERTICALLAYOUT_H

#include "ecs/component.h"

#include <glm/glm.hpp>
#include <string>

namespace dzemikk {
class VerticalLayout : public Component {
  public:
    using Base = Component;

    VerticalLayout() = default;
    VerticalLayout(const VerticalLayout& other) = delete;
    VerticalLayout& operator=(const VerticalLayout& other) = delete;
    VerticalLayout(VerticalLayout&& other) noexcept = delete;
    VerticalLayout& operator=(VerticalLayout&& other) noexcept = delete;
    ~VerticalLayout() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "VerticalLayout";
    }

    void setSpacing(float spacing);
    [[nodiscard]] float getSpacing() const;

    void setChildForceExpandWidth(bool enabled);
    [[nodiscard]] bool getChildForceExpandWidth() const;

    void setChildForceExpandHeight(bool enabled);
    [[nodiscard]] bool getChildForceExpandHeight() const;

    void rebuild();

  private:
    float _spacing = 0.0F;
    bool _childForceExpandWidth = false;
    bool _childForceExpandHeight = true;
};

} // namespace dzemikk

#endif // DZEMIKK_VERTICALLAYOUT_H
