#ifndef DZEMIKK_SOUND_HANDLE_H
#define DZEMIKK_SOUND_HANDLE_H

#include "IAssetHandler.h"

#include <string>

#include<fmod/include/fmod/fmod.hpp>

namespace dzemikk {
    class Sound;

    class SoundHandler : public IAssetHandler {
      public:
        void* load(const std::string& path) override;
        void reload(void* asset, const std::string& path) override;
        void unload(void* asset) override;

        // FOR TEST ONLY - DELETE THIS
        FMOD::System* system;

      private:
        Sound* loadSoundFromFile(const std::string& path);
        void reloadSound(const std::string& path, Sound* sound);
    };

} // namespace dzemikk

#endif // DZEMIKK_SOUND_HANDLE_H