#ifndef DZEMIKK_TEXTURE_HANDLE_H
#define DZEMIKK_TEXTURE_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Texture;

    /**
     * @brief Handles loading, reloading, and unloading of Texture assets.
     *
     * TextureHandler uses stb_image to load image data from disk and creates
     * GPU textures using OpenGL. Loaded textures are wrapped in Texture objects
     * and managed by the engine asset system.
     *
     * Supports hot-reloading and safe lifetime management via AssetHandle.
     */
    class TextureHandler : public IAssetHandler<Texture> {
      public:
        using Handle = AssetHandle<Texture>;
        using Result = AssetResult<Texture>;

        /**
         * @brief Loads a texture from disk.
         *
         * @param path Path to the image file.
         * @return AssetResult containing a valid Texture handle or error.
         */
        Result load(const std::string& path,
                    LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync) override;

        /**
         * @brief Reloads an existing texture.
         *
         * Recreates the underlying GPU texture and updates the Texture object.
         *
         * @param asset Reference to texture handle.
         * @param path Path to the image file.
         * @return True if reload succeeded.
         */
        bool reload(Handle& asset, const std::string& path) override;
        
        /**
         * @brief Unloads a texture from memory.
         *
         * @param asset Texture handle to unload.
         */
        void unload(Handle& asset) override;

      private:
        /**
         * @brief Loads image data from file and creates a Texture object.
         *
         * Uses stb_image to read pixel data and initializes a GPU texture.
         *
         * @param path Path to image file.
         * @param flipVertical Whether to flip image vertically (OpenGL convention).
         * @return Shared pointer to Texture or nullptr on failure.
         */
        static std::shared_ptr<Texture> loadTextureFromFile(const std::string& path,
                                                            bool flipVertical = true);

        /**
         * @brief Reloads texture data into an existing Texture instance.
         *
         * Loads new image data and replaces the underlying GPU texture.
         *
         * @param path Path to image file.
         * @param texture Reference to Texture instance.
         * @return True if reload succeeded.
         */
        static bool reloadTexture(const std::string& path, Texture& texture);
    };

} // namespace dzemikk

#endif // DZEMIKK_TEXTURE_HANDLE_H