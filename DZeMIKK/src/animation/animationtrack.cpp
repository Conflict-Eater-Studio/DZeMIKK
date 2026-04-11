#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

#include <glm/ext/quaternion_common.hpp>
#include "animation/animationtrack.h"
#include "assimp/anim.h"

dzemikk::AnimationTrack::AnimationTrack() {
    _name = "Track";
}
dzemikk::AnimationTrack::AnimationTrack(const aiNodeAnim* track) {
}
const std::string& dzemikk::AnimationTrack::getName() const noexcept {
    return _name;
}
void dzemikk::AnimationTrack::setName(const std::string& name) {
    _name = name;
}
void dzemikk::AnimationTrack::interpolate(float time) {
    if (_keys.empty()) {
#if DZEMIKK_DEV_TOOLS
        spdlog::warn("[AnimationTrack] Animation track {} has no keys!", _name);
#endif
        return;
    }
      if (_keys.size() == 1) {
          _property.set(_keys[0].value);
          return;
      }

     size_t index = findPropertyIndex(time);
     size_t next = index + 1;

     float t1 = _keys[index].time;
     float t2 = _keys[next].time;

     float factor = (time - t1) / (t2 - t1);

     float value = glm::mix(_keys[index].value,
                     _keys[next].value,
                     factor);
    _property.set(value);
}

void dzemikk::AnimationTrack::setProperty(const FloatProperty& property) {
    _property = property;
}
dzemikk::FloatProperty dzemikk::AnimationTrack::getProperty() const {
    return _property;
}
const std::vector<dzemikk::FloatPropertyKey>& dzemikk::AnimationTrack::getKeys() const noexcept {
    return _keys;
}
void dzemikk::AnimationTrack::addKey(FloatPropertyKey key) {
    _keys.push_back(key);
}
void dzemikk::AnimationTrack::setKeys(std::vector<FloatPropertyKey> keys) {
    _keys = keys;
}

size_t dzemikk::AnimationTrack::findPropertyIndex(float time) const {
    for (size_t i = 0; i < _keys.size() - 1; i++) {
        if (time < _keys[i + 1].time)
            return i;
    }
    return _keys.size() - 2;
}
