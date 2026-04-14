#ifndef DZEMIKK_SKYBOX_HANDLE_H
#define DZEMIKK_SKYBOX_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <vector>
#include <memory>

namespace dzemikk {
    class Skybox;

    /**
     * @brief Default file names for skybox cubemap faces.
     *
     * Used to construct full file paths when loading a skybox from a directory.
     * Each file corresponds to one face of the cubemap.
     */
    namespace skyboxConst {
        inline constexpr const char* RIGHT = "/right.png";
        inline constexpr const char* LEFT = "/left.png";
        inline constexpr const char* TOP = "/top.png";
        inline constexpr const char* BOTTOM = "/bottom.png";
        inline constexpr const char* FRONT = "/front.png";
        inline constexpr const char* BACK = "/back.png";
    } // namespace SkyboxConst

    /**
     * @brief Handles loading, reloading, and unloading of Skybox assets.
     *
     * SkyboxHandler loads six textures from a directory and creates
     * a cubemap-based Skybox used by the rendering system.
     *
     * Supports hot-reloading and safe lifetime management via AssetHandle.
     */
    class SkyboxHandler : public IAssetHandler<Skybox> {
      public:
        using Handle = AssetHandle<Skybox>;
        using Result = AssetResult<Skybox>;

        /**
         * @brief Loads a skybox from a directory.
         *
         * Expects six cubemap textures with predefined names
         * (right, left, top, bottom, front, back).
         *
         * @param path Directory containing cubemap textures.
         * @return AssetResult containing a valid Skybox handle or error.
         */
        Result load(const std::string& path) override;
        
        /**
         * @brief Reloads an existing skybox.
         *
         * Updates cubemap textures at runtime (hot-reload).
         *
         * @param asset Reference to skybox handle.
         * @param path Directory containing cubemap textures.
         * @return True if reload succeeded, false otherwise.
         */
        bool reload(Handle& asset, const std::string& path) override;
        
        /**
         * @brief Unloads a skybox from memory.
         *
         * Releases ownership of the skybox resource.
         *
         * @param asset Skybox handle to unload.
         */
        void unload(Handle& asset) override;

      private:
        /**
         * @brief Loads skybox textures and creates a Skybox object.
         *
         * @param path Directory with cubemap textures.
         * @return Shared pointer to Skybox or nullptr on failure.
         */
        static std::shared_ptr<Skybox> loadSkyboxFromFile(const std::string& path);

        /**
         * @brief Reloads textures into an existing skybox instance.
         *
         * @param path Directory with cubemap textures.
         * @param skybox Reference to skybox instance.
         * @return True if reload succeeded.
         */
        static bool reloadSkybox(const std::string& path, Skybox& skybox);

        /**
         * @brief Builds file paths for all cubemap faces.
         *
         * @param path Base directory.
         * @return List of file paths in correct cubemap order.
         */
        static std::vector<std::string> buildFaces(const std::string& path);
    };

} // namespace dzemikk

#endif // DZEMIKK_SKYBOX_HANDLE_H