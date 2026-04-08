#include <animation/animationcurve.h>
void AnimationCurve::addValue(float value) const {
    _values.push_back(value);
}
float AnimationCurve::evaluate(float time) const {

}