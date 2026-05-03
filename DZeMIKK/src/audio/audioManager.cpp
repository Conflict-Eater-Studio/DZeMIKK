#include "audio/audioManager.h"

#include "fmod/fmod.hpp"

#if DZEMIKK_DEV_TOOLS
#include <spdlog/spdlog.h>
#endif

namespace dzemikk {
void AudioManager::initialize() {
    FMOD_RESULT result = FMOD::System_Create(&_system);
    if (result != FMOD_OK) {
        return;
    }
    _system->init(512, FMOD_INIT_NORMAL, nullptr);

    _system->createChannelGroup("Master", &_masterGroup);
    _system->createChannelGroup("Music", &_musicGroup);
    _system->createChannelGroup("SFX", &_sfxGroup);
    _system->createChannelGroup("Ambient", &_ambientGroup);
    _system->createChannelGroup("UI", &_uiGroup);

    _masterGroup->addGroup(_musicGroup, false, nullptr);
    _masterGroup->addGroup(_sfxGroup, false, nullptr);
    _masterGroup->addGroup(_ambientGroup, false, nullptr);
    _masterGroup->addGroup(_uiGroup, false, nullptr);
}

void AudioManager::uninitialize() {
    if (_uiGroup) {
        _uiGroup->release();
        _uiGroup = nullptr;
    }
    if (_ambientGroup) {
        _ambientGroup->release();
        _ambientGroup = nullptr;
    }
    if (_sfxGroup) {
        _sfxGroup->release();
        _sfxGroup = nullptr;
    }
    if (_musicGroup) {
        _musicGroup->release();
        _musicGroup = nullptr;
    }
    if (_masterGroup) {
        _masterGroup->release();
        _masterGroup = nullptr;
    }
    if (_system) {
        _system->close();
        _system->release();
        _system = nullptr;
    }
}

void AudioManager::update(float deltaTime) {
    if (_system) {
        _system->update();
    }
}

FMOD::System* AudioManager::getSystem() {
    return _system;
}

FMOD::ChannelGroup* AudioManager::getMasterGroup() {
    return _masterGroup;
}

FMOD::ChannelGroup* AudioManager::getMusicGroup() {
    return _musicGroup;
}

FMOD::ChannelGroup* AudioManager::getSFXGroup() {
    return _sfxGroup;
}

FMOD::ChannelGroup* AudioManager::getUIGroup() {
    return _uiGroup;
}

FMOD::ChannelGroup* AudioManager::getAmbientGroup() {
    return _ambientGroup;
}

FMOD::ChannelGroup* AudioManager::getGroup(SoundType type) {
    switch (type) {
    case SoundType::Music:
        return _musicGroup;
    case SoundType::SFX:
        return _sfxGroup;
    case SoundType::Ambient:
        return _ambientGroup;
    case SoundType::UI:
        return _uiGroup;
    default:
        return nullptr;
    }
}

FMOD::Channel* AudioManager::play(const Sound& sound, SoundType type, bool looping) {
    FMOD::ChannelGroup* group = getGroup(type);
    FMOD::Sound* fmodSound = sound.getRaw();

    if (!_system) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("AudioManager: _system is null");
#endif
        return nullptr;
    }
    if (!group) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("AudioManager: group is null (type={})", (int)type);
#endif
        return nullptr;
    }
    if (!fmodSound) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("AudioManager: fmodSound is null");
#endif
        return nullptr;
    }

    FMOD_MODE mode = looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    fmodSound->setMode(mode);

    FMOD::Channel* channel = nullptr;
    FMOD_RESULT result = _system->playSound(fmodSound, group, false, &channel);
    if (result != FMOD_OK) {
#if DZEMIKK_DEV_TOOLS
        spdlog::error("AudioManager: playSound failed with {}", (int)result);
#endif
        return nullptr;
    }

    return channel;
}

void AudioManager::stop(FMOD::Channel* channel) {
    if (channel) {
        channel->stop();
    }
}

void AudioManager::stopAll(SoundType type) {
    if (auto* group = getGroup(type)) {
        group->stop();
    }
}

void AudioManager::stopAll() {
    if (_masterGroup) {
        _masterGroup->stop();
    }
}

void AudioManager::setLooping(FMOD::Channel* channel, bool looping) {
    if (channel) {
        FMOD_MODE mode = looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
        channel->setMode(mode);
    }
}
} // namespace dzemikk
