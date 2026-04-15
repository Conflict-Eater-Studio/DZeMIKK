#ifndef DZEMIKK_SOUND_HANDLE_H
#define DZEMIKK_SOUND_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>
#include <fmod/include/fmod/fmod.hpp>

namespace dzemikk {
    class Sound;
    
    /**
     * @brief Handles loading, reloading, and unloading of Sound assets using FMOD.
     *
     * SoundHandler uses an FMOD::System instance to create and manage sound resources.
     * Loaded sounds are wrapped in Sound objects and integrated into the engine asset system.
     *
     * Supports hot-reloading and safe lifetime management via AssetHandle.
     */
    class SoundHandler : public IAssetHandler<Sound> {
      public:
        /**
         * @brief Loads a sound from file.
         *
         * @param path Path to audio file.
         * @return AssetResult containing a valid Sound handle or error.
         */
        Result load(const std::string& path) override;
        
        /**
         * @brief Reloads an existing sound.
         *
         * Recreates internal FMOD sound resource.
         *
         * @param asset Reference to sound handle.
         * @param path Path to audio file.
         * @return True if reload succeeded.
         */
        bool reload(Handle& asset, const std::string& path) override;
        
        /**
         * @brief Unloads a sound.
         *
         * Releases the underlying FMOD sound resource.
         *
         * @param asset Sound handle to unload.
         */
        void unload(Handle& asset) override;

        // FOR TEST ONLY - DELETE THIS
        /**
         * @brief FMOD system used to create sounds.
         *
         * @warning Temporary public field (for testing).
         */
        FMOD::System* system = nullptr;

      private:
        /**
         * @brief Loads a sound and creates a Sound object.
         *
         * @param path Path to audio file.
         * @return Shared pointer to Sound or nullptr on failure.
         */
        [[nodiscard]] std::shared_ptr<Sound> loadSoundFromFile(const std::string& path) const;

        /**
         * @brief Reloads sound data into existing instance.
         *
         * @param path Path to audio file.
         * @param sound Reference to Sound instance.
         * @return True if reload succeeded.
         */
        bool reloadSound(const std::string& path, Sound& sound) const;
    };

} // namespace dzemikk

#endif // DZEMIKK_SOUND_HANDLE_H