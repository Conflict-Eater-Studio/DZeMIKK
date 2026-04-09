#include "animation/animationtrack.h"
dzemikk::AnimationTrack::AnimationTrack(std::string name) : _name(std::move(name)) {

}
const std::string& dzemikk::AnimationTrack::getName() const noexcept {
    return _name;
}
void dzemikk::AnimationTrack::addPositionKey(float time, const glm::vec3& value) {
    PositionKey key = {time, value};
    _positions.push_back(key);
}
void dzemikk::AnimationTrack::addRotationKey(float time, const glm::quat& value) {
    RotationKey key = {time, value};
    _rotations.push_back(key);
}
void dzemikk::AnimationTrack::addScaleKey(float time, const glm::vec3& value) {
    ScaleKey key = {time, value};
    _scales.push_back(key);
}
glm::vec3 dzemikk::AnimationTrack::interpolatePosition(float time) const {
    if (_positions.size() == 1)
        return _positions[0].value;

    size_t index = findPositionIndex(time);
    size_t next = index + 1;

    float t1 = _positions[index].time;
    float t2 = _positions[next].time;

    float factor = (time - t1) / (t2 - t1);

    return glm::mix(_positions[index].value,
                    _positions[next].value,
                    factor);
}
glm::quat dzemikk::AnimationTrack::interpolateRotation(float time) const {
    glm::quat result = glm::slerp(_rotations[0].value, _rotations[1].value, (time - _rotations[0].time) / (_rotations[1].time - _rotations[0].time));
    return result;
}
glm::vec3 dzemikk::AnimationTrack::interpolateScale(float time) const {
    if (_scales.size() == 1)
        return _scales[0].value;

    size_t index = findScaleIndex(time);
    size_t next = index + 1;

    float t1 = _scales[index].time;
    float t2 = _scales[next].time;

    float factor = (time - t1) / (t2 - t1);

    return glm::mix(_scales[index].value,
                    _scales[next].value,
                    factor);
}

size_t dzemikk::AnimationTrack::findPositionIndex(float time) const {
    for (size_t i = 0; i < _positions.size() - 1; i++) {
        if (time < _positions[i + 1].time)
            return i;
    }
    return _positions.size() - 2;
}
size_t dzemikk::AnimationTrack::findRotationIndex(float time) const {
    for (size_t i = 0; i < _rotations.size() - 1; i++) {
        if (time < _rotations[i + 1].time)
            return i;
    }
    return _rotations.size() - 2;
}
size_t dzemikk::AnimationTrack::findScaleIndex(float time) const {
    for (size_t i = 0; i < _scales.size() - 1; i++) {
        if (time < _scales[i + 1].time)
            return i;
    }
    return _scales.size() - 2;
}