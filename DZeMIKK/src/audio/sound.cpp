#include "audio/sound.h"

#include <iostream>

namespace dzemikk {
Sound::~Sound() {
    if (_sound) {
        //_sound->release();
        //_sound = nullptr;
    }
}

void Sound::init(FMOD::Sound* sound) {
    _sound = sound;
}

void Sound::replaceSound(FMOD::Sound* newSound) {
    if (_sound) {
        _sound->release();
    }

    _sound = newSound;
}
} // namespace dzemikk
