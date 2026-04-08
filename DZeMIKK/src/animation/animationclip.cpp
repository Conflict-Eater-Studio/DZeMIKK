#include "animation/animationclip.h"

#include "ecs/components/transform.h"
#include "spdlog/spdlog.h"
namespace dzemikk {
// namespace math {
//     Transform lerp(const Transform& a, const Transform& b, float t) {
//         Transform transform = Transform();
//         glm::vec3 scale = glm::mix(a.getScale(), b.getScale(), t);
//         transform.setScale(scale);
//         return transform;
//     }
//     Pose lerp(const Pose& a, const Pose& b, float t) {
//         Transform blended = lerp(a.transform, b.transform, t);
//         Pose pose = Pose(blended);
//         return pose; // or better memory handling
//     }
// }
AnimationClip::AnimationClip(int frames, int framerate) : _length(frames), _framerate(framerate) {
    }
    int AnimationClip::getLength() const {
        return _length;
    }
    int AnimationClip::getFramerate() const {
        return _framerate;
    }
    void AnimationClip::addCurve(const AnimationCurve& curve) {
        _curves.push_back(curve);
    }


    float AnimationClip::sample(float time) const {
        float frameTime = time * _framerate;
        int frame = (int)frameTime % _length;
        return _curves[0].evaluate(frame);
    // float frameTime = time * _framerate;
        //
        // int frameA = (int)frameTime;
        // int frameB = frameA + 1;
        //
        // frameA = frameA % _length;
        // frameB = frameB % _length;
        //
        // float t = frameTime - frameA;
        // spdlog::info("{} to {}", _poses[frameA].transform.getScale().x,  _poses[frameB].transform.getScale().x);
        //
        // return math::lerp(_poses[frameA], _poses[frameB], t);
    }

    } // namespace dzemikk
