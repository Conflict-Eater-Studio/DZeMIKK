#ifndef DZEMIKK_I_ASSET_HANDLE_H
#define DZEMIKK_I_ASSET_HANDLE_H

#include <string>

namespace dzemikk {

    /**
     * @brief Base interface for all asset handlers in the engine.
     *
     * Defines a common API for loading, reloading, and unloading runtime assets
     *
     * Handlers are responsible for converting file data into engine-specific runtime objects.
     *
     * @note Uses void* to allow generic asset storage across different types.
     * @warning Caller is responsible for correct casting of returned pointers.
     */
    class IAssetHandler {
      public:
        virtual ~IAssetHandler() = default;

        
        IAssetHandler() = default;

        IAssetHandler(const IAssetHandler&) = delete;
        IAssetHandler& operator=(const IAssetHandler&) = delete;

        IAssetHandler(IAssetHandler&&) = delete;
        IAssetHandler& operator=(IAssetHandler&&) = delete;


        /**
         * @brief Loads an asset from disk.
         *
         * @param path Path to asset file.
         * @return void* Pointer to loaded asset.
         */
        virtual void* load(const std::string& path) = 0;
        
        /**
         * @brief Reloads an existing asset from disk.
         *
         * @param asset Pointer to existing asset instance.
         * @param path Path to asset file.
         */
        virtual void reload(void* asset, const std::string& path) = 0;
        
        /**
         * @brief Unloads an asset from memory.
         *
         * @param asset Pointer to asset instance.
         */
        virtual void unload(void* asset) = 0;
    };
} // namespace dzemikk

#endif // DZEMIKK_I_ASSET_HANDLE_H
