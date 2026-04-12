#ifndef DZEMIKK_SKYBOX_HANDLE_H
#define DZEMIKK_SKYBOX_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <vector>

namespace dzemikk {
    class Skybox;

    namespace SkyboxConst {
        inline constexpr const char* RIGHT = "/right.png";
        inline constexpr const char* LEFT = "/left.png";
        inline constexpr const char* TOP = "/top.png";
        inline constexpr const char* BOTTOM = "/bottom.png";
        inline constexpr const char* FRONT = "/front.png";
        inline constexpr const char* BACK = "/back.png";
    } // namespace SkyboxConst

    class SkyboxHandler : public IAssetHandler {
      public:
        void* load(const std::string& path) override;
        void reload(void* asset, const std::string& path) override;
        void unload(void* asset) override;

      private:
        Skybox* loadSkyboxFromFile(const std::string& path);
        void reloadSkybox(const std::string& path, Skybox* skybox);

        std::vector<std::string> buildFaces(const std::string& path);
    };

} // namespace dzemikk

#endif // DZEMIKK_SKYBOX_HANDLE_H