#pragma once

#include <ecs/components/monobehaviour.h>
#include <ecs/gameobject.h>
#include <glm/common.hpp>
#include <ecs/components/ui/imageRenderer.h>

class UIPulseEffect : public dzemikk::MonoBehaviour {
  public:
    void trigger() {
        _active = true;
        _timer = 0.0f;
    }

    void update(double dt) override {
        if (!_active)
            return;

        _timer += dt;

        constexpr float duration = 0.3f;

        float t = glm::clamp(_timer / duration, 0.0f, 1.0f);

        float scaleFactor;

        if (t < 0.5f) {
            scaleFactor = glm::mix(1.0f, 1.2f, t * 2.0f);
        } else {
            scaleFactor = glm::mix(1.2f, 1.0f, (t - 0.5f) * 2.0f);
        }

        getOwner()->rectTransform()->setScale(glm::vec3(scaleFactor));

        auto* img = getOwner()->getComponent<dzemikk::ImageRenderer>();
        if (img) {
            float brightness = 1.0f + 0.4f * std::sin(t * glm::pi<float>());

            img->setColor({brightness, brightness, brightness, 1.0f});
        }

        if (t >= 1.0f) {
            getOwner()->rectTransform()->setScale(glm::vec3(1.0f));

            if (img) {
                img->setColor({1, 1, 1, 1});
            }

            _active = false;
        }
    }

    [[nodiscard]] std::string typeName() const override {
        return "UIPulseEffect";
    }

  private:
    bool _active = false;
    float _timer = 0.0f;
};
