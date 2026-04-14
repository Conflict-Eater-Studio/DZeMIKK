#include "assetManager/soundHandler.h"

#include "assetManager/assetError.h"
#include "audio/sound.h"

#include <iostream>dzemikk::

dzemikk::SoundHandler::Result dzemikk::SoundHandler::load(const std::string& path) {
    auto sound = loadSoundFromFile(path);

    if (!sound) {
        std::cerr << "Failed to load sound: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {sound, AssetError::None};
}

std::shared_ptr<dzemikk::Sound> dzemikk::SoundHandler::loadSoundFromFile(const std::string& path) const {
    if (!system) {
        std::cerr << "FMOD system not initialized!\n";
        return nullptr;
    }

    FMOD::Sound* fmodSound = nullptr;

    FMOD_RESULT result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &fmodSound);

    if (result != FMOD_OK || !fmodSound) {
        std::cerr << "FMOD failed to create sound: " << path << "\n";
        return nullptr;
    }

    auto sound = std::make_shared<Sound>();
    sound->init(fmodSound);

    return sound;
}

bool dzemikk::SoundHandler::reloadSound(const std::string& path, Sound& sound) const {
    if (!system)
        return false;

    FMOD::Sound* newSound = nullptr;

    FMOD_RESULT result = system->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &newSound);

    if (result != FMOD_OK || !newSound) {
        std::cerr << "Failed to reload sound: " << path << "\n";
        return false;
    }

    sound.replaceSound(newSound);
    return true;
}

bool dzemikk::SoundHandler::reload(Handle& asset, const std::string& path) {
    if (!asset)
        return false;

    return reloadSound(path, *asset.get());
}

void dzemikk::SoundHandler::unload(Handle& asset) {
    asset = Handle{};
}