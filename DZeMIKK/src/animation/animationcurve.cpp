#include <animation/animationcurve.h>
namespace dzemikk {
    void AnimationCurve::addValue(float value) {
        _values.push_back(value);
    }
    float AnimationCurve::evaluate(float time) const {
        return _values[static_cast<int>(time)];
    }
}
