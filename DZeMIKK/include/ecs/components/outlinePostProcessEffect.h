#pragma once
#ifndef DZEMIKK_OUTLINEPOSTPROCESSEFFECT_H
#define DZEMIKK_OUTLINEPOSTPROCESSEFFECT_H
#include "postProcessEffect.h"

namespace dzemikk {
class Shader;
class OutlinePostProcessEffect : public PostProcessEffect {
public:
    using Base = PostProcessEffect;

    OutlinePostProcessEffect() = default;
    ~OutlinePostProcessEffect() override = default;

    void bindShaderUniforms(Shader& shader) override;

    [[nodiscard]] std::string typeName() const override;

    void setColor(glm::vec3 color);

private:
    glm::vec3 _color{};
};
}

#endif
