#ifndef DZEMIKK_I_ASSET_HANDLE_H
#define DZEMIKK_I_ASSET_HANDLE_H

#include <string>

namespace dzemikk {
    class IAssetHandler {
      public:
        virtual ~IAssetHandler() = default;

        virtual void* load(const std::string& path) = 0;
        virtual void reload(void* asset, const std::string& path) = 0;
        virtual void unload(void* asset) = 0;
    };
} // namespace dzemikk

#endif // DZEMIKK_I_ASSET_HANDLE_H
