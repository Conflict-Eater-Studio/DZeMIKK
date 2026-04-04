#include "animation/animationclip.h"

#include "animation/pose.h"
#include "ecs/components/transform.h"
namespace dzemikk {
namespace math {
    Transform lerp(const Transform& a, const Transform& b, float t) {
        Transform transform = Transform();
        glm::vec3 scale = glm::mix(a.getScale(), b.getScale(), t);
        transform.setScale(scale);
        return transform;
    }
    Pose lerp(const Pose& a, const Pose& b, float t) {
        Transform blended = lerp(a.transform, b.transform, t);
        Pose pose = Pose(blended);
        return pose; // or better memory handling
    }
}
AnimationClip::AnimationClip(int frames, int framerate) : _length(frames), _framerate(framerate) {
    _poses.reserve(frames);
    }
    int AnimationClip::getLength() const {
        return _length;
    }
    int AnimationClip::getFramerate() const {
        return _framerate;
    }
    void AnimationClip::addPose(const Pose& pose) {
        _poses.push_back(pose);
    }

    Pose AnimationClip::sample(float time) const {
        float frameTime = time * _framerate;

        int frameA = (int)frameTime;
        int frameB = frameA + 1;

        frameA = frameA % _length;
        frameB = frameB % _length;

        float t = frameTime - frameA;

        return math::lerp(_poses[frameA], _poses[frameB], t);
    }

    } // namespace dzemikk
