#pragma once
#include "monoBehaviour.h"

namespace dzemikk {

    class Transform;
    class Mesh;
    class Material;

    class SpriteRenderer : public MonoBehaviour {
    public:
          using Base = MonoBehaviour;

        Mesh* mesh = nullptr;
        Material* material = nullptr;
        Transform* transform = nullptr;

        void start() override;
        void onDestroy() override;
    };

} 