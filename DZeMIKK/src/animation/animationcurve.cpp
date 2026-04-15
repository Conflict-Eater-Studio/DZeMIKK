#include <animation/animationcurve.h>
namespace dzemikk {
    void AnimationCurve::addValue(float value) {
        _values.push_back(value);
    }
    float AnimationCurve::evaluate(float keyframe) const {
        int idx = static_cast<int>(keyframe);
        float currentValue = _values[idx];
        if (idx == _values.size() - 1) return currentValue;
        float nextValue = _values[idx + 1];

        return currentValue + (nextValue - currentValue) * (keyframe - idx);
    }
}
