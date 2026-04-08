#ifndef DZEMIKK_ANIMATIONCURVE_H
#define DZEMIKK_ANIMATIONCURVE_H
#include <vector>
namespace dzemikk {
    class AnimationCurve {
    public:
        AnimationCurve() = default;
        void addValue(float value);
        float evaluate(float time) const;
    private:
        std::vector<float> _values = std::vector<float>();

    };
}
#endif
