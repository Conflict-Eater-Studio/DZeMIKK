#ifndef DZEMIKK_TEXTURE_HANDLE_H
#define DZEMIKK_TEXTURE_HANDLE_H

#include "IAssetHandler.h"

#include <string>

namespace dzemikk {
class Texture;

class TextureHandler : public IAssetHandler {
  public:
    void* load(const std::string& path) override;
    void reload(void* asset, const std::string& path) override;
    void unload(void* asset) override;

  private:
    Texture* loadTextureFromFile(const std::string& id, bool flipVertical = true);
    void reloadTexture(const std::string& path, Texture* texture);
};

} // namespace dzemikk

#endif // DZEMIKK_TEXTURE_HANDLE_H