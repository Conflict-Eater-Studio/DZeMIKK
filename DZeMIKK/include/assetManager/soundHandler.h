#ifndef DZEMIKK_SOUND_HANDLE_H
#define DZEMIKK_SOUND_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>
#include <fmod/include/fmod/fmod.hpp>

namespace dzemikk {
    class Sound;
    
    /**
     * @brief Handles loading and management of Sound assets using FMOD.
     *
     * Uses FMOD::System to create sounds from files and wraps them in Sound objects.
     *
     * @note Requires a valid FMOD::System instance.
     * @note Loaded sounds must be released via unload().
     * @warning Uses void* interface — requires casting to Sound*.
     */
    class SoundHandler : public IAssetHandler {
      public:
        /**
         * @brief Loads a sound from file.
         *
         * @param path Path to the audio file.
         * @return void* Pointer to Sound.
         */
        void* load(const std::string& path) override;
        
        /**
         * @brief Reloads an existing sound.
         *
         * Creates a new FMOD::Sound and replaces the internal handle.
         *
         * @param asset Pointer to Sound (as void*).
         * @param path Path to the audio file.
         */
        void reload(void* asset, const std::string& path) override;
        
        /**
         * @brief Unloads a sound.
         *
         * @param asset Pointer to Sound (as void*).
         */
        void unload(void* asset) override;

        // FOR TEST ONLY - DELETE THIS
        /**
         * @brief FMOD system used to create sounds.
         *
         * @warning Temporary public field (for testing).
         */
        FMOD::System* system;

      private:
        /**
         * @brief Loads a sound and creates a Sound object.
         */
        [[nodiscard]] std::unique_ptr<dzemikk::Sound> loadSoundFromFile(const std::string& path) const;
        
        /**
         * @brief Reloads sound data into an existing object.
         */
        void reloadSound(const std::string& path, Sound* sound) const;
    };

} // namespace dzemikk

#endif // DZEMIKK_SOUND_HANDLE_H