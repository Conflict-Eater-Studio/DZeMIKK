#pragma once

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/components/monoBehaviour.h"

namespace game {

class LogoComponent : public dzemikk::MonoBehaviour {
  public:
    void start() override;
    void update(double deltaTime) override;

    [[nodiscard]] std::string typeName() const override;

    void onFinished();

  private:
    enum class State { FadeIn, Wait, FadeOut, Finished };

    dzemikk::ImageRenderer* _imageRenderer = nullptr;

    State _state = State::FadeIn;

    double _timer = 0.0;

    double _fadeInDuration = 1.0;
    double _waitDuration = 1.0;
    double _fadeOutDuration = 1.0;
};

} // namespace dzemikk