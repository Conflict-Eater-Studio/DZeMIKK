#include "assetManager/fontHandler.h"
#include "assetManager/assetError.h"

#include "renderer/font.h"
#include <iostream>


dzemikk::FontHandler::Result dzemikk::FontHandler::load(const std::string& path) {
    auto font = loadFontFromFile(path);

    if (!font) {
        std::cerr << "Failed to load font: " << path << "\n";
        return {nullptr, AssetError::LoadFailed};
    }

    return {font, AssetError::None};
}

std::shared_ptr<dzemikk::Font> dzemikk::FontHandler::loadFontFromFile(const std::string& path) {
    auto font = std::make_shared<Font>();

    if (!font->load(path)) {
        return nullptr;
    }

    return font;
}

bool dzemikk::FontHandler::reload(Handle& asset, const std::string& path) {
    if (!asset) {
        return false;
    }

    return reloadFont(path, *asset.get());
}

bool dzemikk::FontHandler::reloadFont(const std::string& path, Font& font) {
    if (!font.load(path)) {
        std::cerr << "Failed to reload font: " << path << "\n";
        return false;
    }

    return true;
}

void dzemikk::FontHandler::unload(Handle& asset) {
    asset = Handle(); 
}


