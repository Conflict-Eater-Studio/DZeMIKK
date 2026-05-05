#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H

#include <memory>
#include <vector>
#include "IAnimationTrack.h"

struct aiAnimation;
namespace dzemikk {
class FloatTrack;
class VectorTrack;
class QuaternionTrack;
class Transform;

/**
 * @brief Represents an animation clip consisting of multiple animation tracks.
 *
 * AnimationClip stores time-based animation data that can be applied to various
 * properties (e.g., transform position, rotation). Each track targets a specific
 * property and is sampled during playback.
 */
class AnimationClip {
public:

    /**
     * @brief Default constructor.
     */
    AnimationClip() = default;

    /**
     * @brief Constructs an animation clip with duration and framerate.
     *
     * @param durationInTicks Total duration of the clip (in ticks).
     * @param tickPerSecond Playback framerate (ticks per second).
     */
    AnimationClip(float durationInTicks, float tickPerSecond);
    /**
     * @brief Gets the total duration of the animation clip.
     *
     * @return Duration in ticks.
     */
    [[nodiscard]] float getTickDuration() const;

    /**
     * @brief Gets the framerate of the animation clip.
     *
     * @return Framerate (ticks per second).
     */
    [[nodiscard]] float getTickrate() const;

    /**
     * @brief Creates and adds a new animation track.
     * @return Pointer to the newly created track (owned by this clip)
     *
     * The track is initially empty. The returned pointer remains valid
     * as long as the track exists within this clip.
     */
    FloatTrack* addFloatTrack();
    VectorTrack* addVectorTrack();
    QuaternionTrack* addQuaternionTrack();

    /**
    * @brief Samples the animation clip at a given time.
    *
    * Updates all tracks and applies their values to bound properties.
    *
    * @param timeInSeconds Current time within the animation (in seconds).
    */
    void apply(float timeInSeconds) const;
    static std::shared_ptr<AnimationClip> fromAssimp(aiAnimation* animation);
private:
    std::vector<std::unique_ptr<IAnimationTrack>> _tracks;
    float _durationInTicks = 0;
    float _ticksPerSecond = 0;
};

}

#endif