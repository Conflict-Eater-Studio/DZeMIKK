#ifndef DZEMIKK_FONT_HANDLE_H
#define DZEMIKK_FONT_HANDLE_H

#include "IAssetHandler.h"

#include <string>

namespace dzemikk {
class Font;

class FontHandler : public IAssetHandler {
  public:
    void* load(const std::string& path) override;
    void reload(void* asset, const std::string& path) override;
    void unload(void* asset) override;

  private:
    Font* loadFontFromFile(const std::string& path);
    void reloadFont(const std::string& path, Font* font);
};

} // namespace dzemikk

#endif // DZEMIKK_FONT_HANDLE_H