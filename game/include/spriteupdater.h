#ifndef DZEMIKK_SPRITEUPDATER_H
#define DZEMIKK_SPRITEUPDATER_H
#include "ecs/components/monoBehaviour.h"
#include "ecs/components/transform.h"

namespace dzemikk {
    class SpriteUpdater: public MonoBehaviour {
    public:
        using Base = MonoBehaviour;

        Transform* transform = nullptr;
        float time = 0.0f;

        void update(double deltaTime) override {
            time += deltaTime;

            float scaleX = 0.5f + 0.5f * sin(time);
            float scaleY = 1.0f;

            transform->setScale(glm::vec3(scaleX, scaleY, 1.0f));
        }

        [[nodiscard]] std::string typeName() const override {
            return "SpriteUpdater";
        };
    };
}


#endif
