#pragma once
#ifndef DZEMIKK_SPRITE_ANIMATION_H
#define DZEMIKK_SPRITE_ANIMATION_H

#include "assetManager/assetHandle.h"
#include "ecs/component.h"

#include <string>
#include <unordered_map>

namespace dzemikk {
class ImageRenderer;
class Texture;
class SpriteAnimation : public Component {
  public:
    using Base = Component;

    SpriteAnimation() = default;

    [[nodiscard]] std::string typeName() const override {
        return "SpriteAnimation";
    }

    [[nodiscard]] std::string getAnimationName() const;
    [[nodiscard]] float getFps() const;
    [[nodiscard]] std::unordered_map<unsigned int, AssetHandle<Texture>>& getFrames();
    [[nodiscard]] bool isPlaying() const;

    ImageRenderer* getRenderer();

    void setAnimationName(const std::string& name);
    void setFps(float fps);
    void setFrames(const std::unordered_map<unsigned int, AssetHandle<Texture>>& frames);

    void play();
    void stop();
    void pause();

    void update(float deltaTime);

  private:
    std::string _animationName = "Anim";
    float _fps = 24.0F;

    bool _isPlaying = false;

    unsigned int _currentFrameIndex = 0;
    float _currentTime = 0.0F;
    float _frameDuration = 1.0F / _fps;

    ImageRenderer* _renderer = nullptr;
    std::unordered_map<unsigned int, AssetHandle<Texture>> _frames;
};
} // namespace dzemikk

#endif // DZEMIKK_SPRITE_ANIMATION_H
