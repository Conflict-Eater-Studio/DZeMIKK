#ifndef DZEMIKK_FONT_HANDLE_H
#define DZEMIKK_FONT_HANDLE_H

#include "IAssetHandler.h"

#include <string>
#include <memory>

namespace dzemikk {
class Font;

/**
 * @brief Handles loading, reloading, and unloading of Font assets.
 *
 * FontHandler is responsible for converting font files into runtime Font objects
 * and integrating them into the engine asset system.
 *
 * Supports hot-reload and safe asset lifetime management via AssetHandle.
 */
class FontHandler : public IAssetHandler<Font> {
  public:
    using Handle = AssetHandle<Font>;
    using Result = AssetResult<Font>;

    /**
     * @brief Loads a font asset from the given file path.
     *
     * @param path Path to the font file.
     * @return AssetResult containing a valid Font handle on success,
     *         or an error state on failure.
     */
    Result load(const std::string& path, LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync) override;

    /**
     * @brief Reloads an already loaded font asset from disk.
     *
     * Used for hot-reloading during development or runtime updates.
     *
     * @param asset Reference to the existing font handle.
     * @param path Path to the font file.
     * @return True if reload succeeded, false otherwise.
     */
    bool reload(Handle& asset, const std::string& path) override;
        
    /**
     * @brief Unloads a font asset from memory.
     *
     * @param asset Reference to the font handle to be unloaded.
     */
    void unload(Handle& asset) override;

    private:
    /**
     * @brief Loads a font and creates a Font object.
     */
      static std::shared_ptr<Font>
      loadFontFromFile(const std::string& path,
                       LoadExecutionMode loadExecutionMode = LoadExecutionMode::Sync);

    /**
     * @brief Reloads font data into an existing Font object.
     */
    static bool reloadFont(const std::string& path, Font& font);
};

} // namespace dzemikk

#endif // DZEMIKK_FONT_HANDLE_H