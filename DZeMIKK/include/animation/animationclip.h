#ifndef TUL_PBL_DZEMIKK_ANIMATIONCLIP_H
#define TUL_PBL_DZEMIKK_ANIMATIONCLIP_H
namespace dzemikk {
    class Pose;
    class AnimationClip {
    public:
        float durationInTicks;
        float ticksPerSecond;

        void sample(float timeInTicks, Pose& outPose);
};
};
#endif // TUL_PBL_DZEMIKK_ANIMATIONCLIP_H
