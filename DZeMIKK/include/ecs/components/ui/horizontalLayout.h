#ifndef DZEMIKK_HORIZONTALLAYOUT_H
#define DZEMIKK_HORIZONTALLAYOUT_H

#include "ecs/component.h"

#include <glm/glm.hpp>
#include <string>

namespace dzemikk {

class HorizontalLayout : public Component {
  public:
    using Base = Component;

    HorizontalLayout() = default;
    HorizontalLayout(const HorizontalLayout& other) = delete;
    HorizontalLayout& operator=(const HorizontalLayout& other) = delete;
    HorizontalLayout(HorizontalLayout&& other) noexcept = delete;
    HorizontalLayout& operator=(HorizontalLayout&& other) noexcept = delete;
    ~HorizontalLayout() override = default;

    [[nodiscard]] std::string typeName() const override {
        return "HorizontalLayout";
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
    bool _childForceExpandWidth = true;
    bool _childForceExpandHeight = false;
};

} // namespace dzemikk

#endif // DZEMIKK_HORIZONTALLAYOUT_H