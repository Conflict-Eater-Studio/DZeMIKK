#ifndef DZEMIKK_FONT_HANDLE_H
#define DZEMIKK_FONT_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
class Font;

/**
    * @brief Asset handler responsible for loading and managing Font resources.
    *
    * Loads font files into Font objects and provides runtime reload support.
    * Uses Font::load() for parsing and initializing font data.
    *
    * @note Loaded fonts are heap-allocated and must be released via unload().
    * @warning Uses void* interface — requires casting to Font*.
    */
class FontHandler : public IAssetHandler {
    public:
    /**
        * @brief Loads a font from file.
        *
        * @param path Path to the font file.
        * @return void* Pointer to Font, or nullptr if loading failed.
        */
    void* load(const std::string& path) override;

    /**
        * @brief Reloads an existing font from file.
        *
        * @param asset Pointer to Font (as void*).
        * @param path Path to the font file.
        */
    void reload(void* asset, const std::string& path) override;
        
    /**
        * @brief Unloads a font from memory.
        *
        * @param asset Pointer to Font (as void*).
        */
    void unload(void* asset) override;

    private:
    /**
        * @brief Loads a font and creates a Font object.
        */
    static std::unique_ptr<dzemikk::Font> loadFontFromFile(const std::string& path);

    /**
        * @brief Reloads font data into an existing Font object.
        */
    static void reloadFont(const std::string& path, Font* font);
};

} // namespace dzemikk

#endif // DZEMIKK_FONT_HANDLE_H