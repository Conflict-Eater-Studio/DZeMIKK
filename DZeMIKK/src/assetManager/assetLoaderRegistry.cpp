#include "assetManager/assetLoaderRegistry.h"

void dzemikk::AssetLoaderRegistry::clear() {
    _handlers.clear();
}

dzemikk::IAssetHandlerBase* dzemikk::AssetLoaderRegistry::getByType(std::type_index type) const {
    auto it = _handlers.find(type);
    if (it == _handlers.end()) {
        return nullptr;
    }

    return it->second.get();
}

bool dzemikk::AssetLoaderRegistry::contains(std::type_index type) const {
    return _handlers.contains(type);
}