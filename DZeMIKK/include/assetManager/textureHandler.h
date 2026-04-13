#ifndef DZEMIKK_TEXTURE_HANDLE_H
#define DZEMIKK_TEXTURE_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
    class Texture;

    /**
     * @brief Asset handler responsible for loading and managing Texture resources.
     *
     * Uses stb_image to load image data from disk and OpenGL to create GPU textures.
     * Returned assets are raw Texture pointers cast to void*, following the IAssetHandler
     * interface.
     *
     * @note Ownership: loaded textures are heap-allocated and must be released via unload().
     * @warning Uses void* interface — caller must ensure correct casting to Texture*.
     */
    class TextureHandler : public IAssetHandler {
      public:
        /**
         * @brief Loads a texture from disk.
         *
         * Internally uses stb_image to read pixel data and creates a Texture object
         * initialized with OpenGL texture data.
         *
         * @param path Path to the image file.
         * @return void* Pointer to Texture, or nullptr if loading failed.
         */
        void* load(const std::string& path) override;

        
        /**
         * @brief Reloads an existing texture with new data from disk.
         *
         * Creates a new OpenGL texture and replaces the internal GPU handle
         * of the existing Texture object.
         *
         * @param asset Pointer to an existing Texture (as void*).
         * @param path Path to the image file.
         */
        void reload(void* asset, const std::string& path) override;
        
        /**
         * @brief Unloads a texture from memory.
         *
         * Deletes the Texture object. Assumes the pointer was created by load().
         *
         * @param asset Pointer to Texture (as void*).
         */
        void unload(void* asset) override;

      private:
        /**
         * @brief Loads image data from file and creates a Texture object.
         *
         * Uses stb_image to read pixel data, optionally flipping it vertically,
         * then initializes a Texture with the loaded data.
         *
         * @param path Path to the image file.
         * @param flipVertical Whether to flip image vertically (OpenGL convention).
         * @return Texture* New Texture instance, or nullptr on failure.
         */
        static std::unique_ptr<dzemikk::Texture> loadTextureFromFile(const std::string& id,
                                                                  bool flipVertical = true);
        
        /**
         * @brief Reloads texture data into an existing Texture object.
         *
         * Loads new pixel data and creates a new OpenGL texture object.
         * The old GPU texture is replaced via Texture::replaceTexture().
         *
         * @param path Path to the image file.
         * @param texture Texture object to update.
         *
         * @note Generates mipmaps and sets default filtering/wrapping.
         */
        static void reloadTexture(const std::string& path, Texture* texture);
    };

} // namespace dzemikk

#endif // DZEMIKK_TEXTURE_HANDLE_H