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
 * properties (e.g. transform position, rotation). Each track targets a specific
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
     * @param duration Total duration of the clip (in seconds).
     * @param framerate Playback framerate (ticks per second).
     */
    AnimationClip(float duration, float framerate);
    /**
     * @brief Gets the total duration of the animation clip.
     *
     * @return Duration in ticks.
     */
    float getDuration() const;

    /**
     * @brief Gets the framerate of the animation clip.
     *
     * @return Framerate (ticks per second).
     */
    float getFramerate() const;

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

    void fromAssimp(aiAnimation* animation);

private:
    /// Collection of animation tracks.
    std::vector<std::unique_ptr<IAnimationTrack>> _tracks;

    /// Total duration of the animation (in seconds).
    float _duration = 0;

    /// Playback framerate (ticks per second).
    float _framerate = 0;
};

}

#endif