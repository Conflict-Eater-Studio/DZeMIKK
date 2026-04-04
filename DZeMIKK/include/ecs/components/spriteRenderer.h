#pragma once
#include "monoBehaviour.h"

#include <string>

namespace dzemikk {

class Transform;
class Mesh;
class Material;

class SpriteRenderer : public MonoBehaviour {
  public:
    Mesh* mesh = nullptr;
    Material* material = nullptr;
    Transform* transform = nullptr;

    [[nodiscard]] std::string typeName() const override {
        return "SpriteRenderer";
    };

    void start() override;
    void onDestroy() override;
};

} // namespace dzemikk