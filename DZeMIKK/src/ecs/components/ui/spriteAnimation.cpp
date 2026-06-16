#include "ecs/components/ui/spriteAnimation.h"

#include "ecs/components/ui/imageRenderer.h"
#include "ecs/gameobject.h"

namespace dzemikk {
std::string SpriteAnimation::getAnimationName() const {
    return _animationName;
}

float SpriteAnimation::getFps() const {
    return _fps;
}

ImageRenderer* SpriteAnimation::getRenderer() {
    if (!_renderer) {
        _renderer = getOwner()->getComponent<ImageRenderer>();
    }
    return _renderer;
}

bool SpriteAnimation::isPlaying() const {
    return _isPlaying;
}

void SpriteAnimation::setAnimationName(const std::string& name) {
    _animationName = name;
}

void SpriteAnimation::setFrames(
    const std::unordered_map<unsigned int, AssetHandle<Texture>>& frames) {
    _frames = frames;
}

void SpriteAnimation::setFps(float fps) {
    _fps = fps;
    _frameDuration = 1.0F / _fps;
}

void SpriteAnimation::play() {
    _isPlaying = true;
}

void SpriteAnimation::stop() {
    _isPlaying = false;
    _currentFrameIndex = 0;
}

void SpriteAnimation::pause() {
    _isPlaying = false;
}

void SpriteAnimation::update(float deltaTime) {
    if (_frames.size() == 0 || !getRenderer() || !_isPlaying) {
        return;
    }

    if (_currentTime >= _frameDuration) {
        if (_currentFrameIndex == _frames.size()) {
            _currentFrameIndex = 0;
        }

        _renderer->setTexture(_frames[_currentFrameIndex++]);

        _currentTime -= _frameDuration;
    } else {
        _currentTime += deltaTime;
    }
}

} // namespace dzemikk
