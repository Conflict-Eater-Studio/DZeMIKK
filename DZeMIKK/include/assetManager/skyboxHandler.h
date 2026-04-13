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
     * @brief Handles loading and management of Skybox (cubemap) assets.
     *
     * Loads six textures from a directory and creates a cubemap-based Skybox.
     *
     * @note Expects specific file names (right, left, top, bottom, front, back).
     * @note Loaded skyboxes must be released via unload().
     * @warning Uses void* interface — requires casting to Skybox*.
     */
    class SkyboxHandler : public IAssetHandler {
      public:
        /**
         * @brief Loads a skybox from directory.
         *
         * @param path Path to directory containing cubemap textures.
         * @return void* Pointer to Skybox.
         */
        void* load(const std::string& path) override;
        
        /**
         * @brief Reloads an existing skybox.
         *
         * @param asset Pointer to Skybox (as void*).
         * @param path Path to directory with textures.
         */
        void reload(void* asset, const std::string& path) override;
        
        /**
         * @brief Unloads a skybox.
         *
         * @param asset Pointer to Skybox (as void*).
         */
        void unload(void* asset) override;

      private:
        /**
         * @brief Loads skybox textures and creates a Skybox object.
         */
        static std::unique_ptr<dzemikk::Skybox> loadSkyboxFromFile(const std::string& path);
        
        /**
         * @brief Reloads textures into an existing skybox.
         */
        static void reloadSkybox(const std::string& path, Skybox* skybox);

        /**
         * @brief Builds file paths for all cubemap faces.
         */
        static std::vector<std::string> buildFaces(const std::string& path);
    };

} // namespace dzemikk

#endif // DZEMIKK_SKYBOX_HANDLE_H