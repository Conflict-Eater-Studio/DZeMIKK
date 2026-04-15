#ifndef DZEMIKK_UITEXT_RENDERER_H
#define DZEMIKK_UITEXT_RENDERER_H

#include "ecs/component.h"
#include "ecs/components/ui/rectTransform.h"

#include <glm/ext/vector_float3.hpp>
#include <string>

namespace dzemikk {
class Font;

class UITextRenderer : public Component {
  public:
    enum class HorizontalAlign { Left, Center, Right };

    enum class VerticalAlign { Bottom, Middle, Top };

    std::string text = "Hello";
    float scale = 1.0F;
    glm::vec3 color = glm::vec3(1.0F);
    HorizontalAlign horizontalAlign = HorizontalAlign::Left;
    VerticalAlign verticalAlign = VerticalAlign::Bottom;

    Font* font = nullptr;

    [[nodiscard]] bool isValid() const {
        return font != nullptr && !text.empty();
    }

    [[nodiscard]] std::string typeName() const override {
        return "UITextRenderer";
    };
};
} // namespace dzemikk

#endif // DZEMIKK_UITEXT_RENDERER_H