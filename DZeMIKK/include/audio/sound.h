#ifndef DZEMIKK_SOUND_H
#define DZEMIKK_SOUND_H

#include <fmod/include/fmod/fmod.hpp>
#include <string>

namespace dzemikk {

/**
 * @brief Wrapper for FMOD sound resource.
 */
class Sound {
  public:
    Sound() = default;
    ~Sound();

#pragma region Disable copy/move

    Sound(const Sound&) = delete;
    Sound(Sound&&) noexcept = delete;
    Sound& operator=(const Sound&) = delete;
    Sound& operator=(Sound&&) noexcept = delete;

#pragma endregion

    void init(FMOD::Sound* sound);

    void replaceSound(FMOD::Sound* newSound);

    [[nodiscard]] FMOD::Sound* getRaw() const {
        return _sound;
    }

  private:
    FMOD::Sound* _sound = nullptr;
};

} // namespace dzemikk

#endif
