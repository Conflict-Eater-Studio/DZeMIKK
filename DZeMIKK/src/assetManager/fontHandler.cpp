#include "assetManager/fontHandler.h"

#include "renderer/font.h"
#include <iostream>

void* dzemikk::FontHandler::load(const std::string& path) {
    return loadFontFromFile(path).release();
}

std::unique_ptr<dzemikk::Font> dzemikk::FontHandler::loadFontFromFile(const std::string& path) {
    auto font = std::make_unique<dzemikk::Font>();

    if (!font->load(path)) {
        std::cerr << "Failed to load font: " << path << "\n";
        return nullptr;
    }

    return font;
}

void dzemikk::FontHandler::reload(void* asset, const std::string& path) {
    reloadFont(path, static_cast<Font*>(asset));
}

void dzemikk::FontHandler::reloadFont(const std::string& path, dzemikk::Font* font) {
    if (!font->load(path)) {
        std::cerr << "Failed to reload font: " << path << "\n";
    }
}

void dzemikk::FontHandler::unload(void* asset) {
    delete static_cast<Font*>(asset);
}

