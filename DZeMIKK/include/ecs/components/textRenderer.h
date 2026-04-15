#ifndef DZEMIKK_TEXT_RENDERER_H
#define DZEMIKK_TEXT_RENDERER_H

#include "../component.h"

#include <glm/ext/vector_float3.hpp>
#include <string>


namespace dzemikk {
class Font;

class TextRenderer : public Component {
  public:
    std::string text = "Hello";
    float scale = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);

    Font* font = nullptr;

    bool isValid() const {
        return font != nullptr && !text.empty();
    }

    [[nodiscard]] std::string typeName() const override {
        return "TextRenderer";
    };
};

} // namespace dzemikk

#endif // DZEMIKK_TEXT_RENDERER_H