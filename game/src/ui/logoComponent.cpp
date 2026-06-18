#include "ui/logoComponent.h"
#include <ecs/gameobject.h>
#include <ecs/scene.h>


void game::LogoComponent::start() {
    _imageRenderer = getOwner()->getComponent<dzemikk::ImageRenderer>();

    if (_imageRenderer != nullptr) {
        _imageRenderer->setColor({1.0F, 1.0F, 1.0F, 0.0F});
    }
}

void game::LogoComponent::update(double deltaTime) {
    if (_imageRenderer == nullptr) {
        return;
    }

    _timer += deltaTime;

    switch (_state) {
    case State::FadeIn: {
        float t = static_cast<float>(_timer / _fadeInDuration);

        if (t >= 1.0f) {
            t = 1.0f;
            _state = State::Wait;
            _timer = 0.0;
        }

        _imageRenderer->setColor({1.0F, 1.0F, 1.0F, (255.0f * t) / 255});
        break;
    }

    case State::Wait: {
        _imageRenderer->setColor({1.0F, 1.0F, 1.0F, 1.0F});

        if (_timer >= _waitDuration) {
            _state = State::FadeOut;
            _timer = 0.0;
        }
        break;
    }

    case State::FadeOut: {
        float t = static_cast<float>(_timer / _fadeOutDuration);

        if (t >= 1.0f) {
            t = 1.0f;

            _imageRenderer->setColor({1.0F, 1.0f, 1.0F, 0.0F});

            _state = State::Finished;
            onFinished();
            return;
        }

        _imageRenderer->setColor({1.0F, 1.0F, 1.0F, (255.0f * (1.0f - t)) / 255});
        break;
    }

    case State::Finished:
        break;
    }
}

std::string game::LogoComponent::typeName() const {
    return "LogoComponent";
}

void game::LogoComponent::onFinished() {
    auto menu = getOwner()->getScene()->findGameObjectByName("VerticalLayout");
    menu->enabled(true);

    getOwner()->enabled(false);

    auto title = getOwner()->getScene()->findGameObjectByName("Title");
    title->enabled(true);
}
