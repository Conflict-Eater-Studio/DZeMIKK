#pragma once
#ifndef DZEMIKK_ANIMATIONCLIP_H
#define DZEMIKK_ANIMATIONCLIP_H

#include <memory>
#include <vector>

namespace dzemikk {
class AnimationTrack;
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
     * @param duration Total duration of the clip (in ticks).
     * @param framerate Playback framerate (ticks per second).
     */
    AnimationClip(int duration, int framerate);

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
    int getFramerate() const;

    /**
     * @brief Creates and adds a new animation track.
     * @return Pointer to the newly created track (owned by this clip)
     *
     * The track is initially empty. The returned pointer remains valid
     * as long as the track exists within this clip.
     */
    AnimationTrack* addTrack();

    /**
    * @brief Samples the animation clip at a given time.
    *
    * Updates all tracks and applies their values to bound properties.
    *
    * @param timeInSeconds Current time within the animation (in seconds).
    */
    void sample(float timeInSeconds) const;

private:
    /// Collection of animation tracks (non-owning).
    std::vector<std::unique_ptr<AnimationTrack>> _tracks;

    /// Total duration of the animation (in ticks).
    int _duration = 0;

    /// Playback framerate (ticks per second).
    int _framerate = 0;
};

}

#endif