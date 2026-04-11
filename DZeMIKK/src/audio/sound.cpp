#include "audio/sound.h"

#include <iostream>

dzemikk::Sound::~Sound() {
    if (_sound) {
        _sound->release();
        _sound = nullptr;
    }
}

void dzemikk::Sound::init(FMOD::Sound* sound) {
    _sound = sound;
}

void dzemikk::Sound::play(FMOD::System* system) {
    if (!_sound || !system)
        return;

    FMOD::Channel* channel = nullptr;
    system->playSound(_sound, nullptr, false, &channel);
}
