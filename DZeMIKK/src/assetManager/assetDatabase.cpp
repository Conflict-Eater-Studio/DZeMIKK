#include "assetManager/assetDatabase.h"

void dzemikk::AssetDatabase::remove(const std::string& path) {
    auto it = _assets.find(path);
    if (it == _assets.end()) {
        return;
    }

    _assets.erase(it);

#if DZEMIKK_DEV_TOOLS
    spdlog::info("[AssetDatabase] Removed: {}", path);
#endif
}

void dzemikk::AssetDatabase::clear() {
    _assets.clear();
}

std::type_index dzemikk::AssetDatabase::getType(const std::string& path) const {
    auto it = _assets.find(path);
    if (it == _assets.end()) {
        return typeid(void);
    }

    return it->second.type;
}

std::shared_ptr<void> dzemikk::AssetDatabase::getRaw(const std::string& path) const{
    auto it = _assets.find(path);
    if (it == _assets.end()) {
        return nullptr;
    }

    return it->second.handle;
}

bool dzemikk::AssetDatabase::contains(const std::string& path) const {
    return _assets.contains(path);
}