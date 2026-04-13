#include "assetManager/soundHandler.h"

#include "audio/sound.h"
#include <iostream>

void* dzemikk::SoundHandler::load(const std::string& path) {
    return loadSoundFromFile(path).release();
}

void dzemikk::SoundHandler::reload(void* asset, const std::string& path) {
    reloadSound(path, static_cast<Sound*>(asset));
}

std::unique_ptr<dzemikk::Sound> dzemikk::SoundHandler::loadSoundFromFile(const std::string& path) const{
    FMOD::Sound* fmodSound = nullptr;

    FMOD_RESULT result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &fmodSound);

    if (result != FMOD_OK || !fmodSound) {
        return nullptr;
    }

    auto sound = std::make_unique<dzemikk::Sound>();
    sound->init(fmodSound);

    return sound;
}

void dzemikk::SoundHandler::reloadSound(const std::string& path, dzemikk::Sound* sound) const{
    FMOD::Sound* newSound = nullptr;

    FMOD_RESULT result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &newSound);

    if (result != FMOD_OK || !newSound) {
        std::cerr << "Failed to reload sound: " << path << "\n";
        return;
    }

    sound->replaceSound(newSound);
}

void dzemikk::SoundHandler::unload(void* asset) {
    delete static_cast<Sound*>(asset);
}
