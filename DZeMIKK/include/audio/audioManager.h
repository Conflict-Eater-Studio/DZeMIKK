#ifndef DZEMIKK_AUDIO_MANAGER_H
#define DZEMIKK_AUDIO_MANAGER_H

#include "audio/sound.h"
#include "core/iEngineModule.h"

#include <fmod/include/fmod/fmod.hpp>

namespace dzemikk {
class AudioManager : public IEngineModule {
  public:
    enum class SoundType : uint8_t { Music, SFX, Ambient, UI };

    AudioManager() = default;
    AudioManager(const AudioManager& other) = delete;
    AudioManager(AudioManager&& other) noexcept = delete;
    AudioManager& operator=(const AudioManager& other) = delete;
    AudioManager& operator=(AudioManager&& other) noexcept = delete;
    ~AudioManager() override = default;

    void initialize() override;
    void uninitialize() override;

    void update(float deltaTime);

    FMOD::Channel* play(const Sound& sound, SoundType type, bool looping = false);
    static void stop(FMOD::Channel* channel);
    void stopAll(SoundType type);
    void stopAll();

    static void setLooping(FMOD::Channel* channel, bool looping);

    FMOD::System* getSystem();
    FMOD::ChannelGroup* getMasterGroup();
    FMOD::ChannelGroup* getMusicGroup();
    FMOD::ChannelGroup* getSFXGroup();
    FMOD::ChannelGroup* getUIGroup();
    FMOD::ChannelGroup* getAmbientGroup();

  private:
    FMOD::ChannelGroup* getGroup(SoundType type);

    FMOD::System* _system{nullptr};
    FMOD::ChannelGroup* _masterGroup{nullptr};
    FMOD::ChannelGroup* _musicGroup{nullptr};
    FMOD::ChannelGroup* _ambientGroup{nullptr};
    FMOD::ChannelGroup* _sfxGroup{nullptr};
    FMOD::ChannelGroup* _uiGroup{nullptr};
};
} // namespace dzemikk

#endif // DZEMIKK_AUDIO_MANAGER_H
