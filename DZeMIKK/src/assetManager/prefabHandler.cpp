#include "assetManager/prefabHandler.h"

#include "assetManager/assetmanager.h"

namespace dzemikk {

PrefabHandler::PrefabHandler(AssetManager* assetManager) : _assetManager(assetManager) {}

PrefabHandler::Result PrefabHandler::load(const std::string& path, LoadExecutionMode) {

    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }

    nlohmann::json json;
    file >> json;

    return Result(std::make_shared<nlohmann::json>(std::move(json)));
}

bool PrefabHandler::reload(Handle& asset, const std::string& path) {

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json json;
    file >> json;

    asset = Handle(std::make_shared<nlohmann::json>(std::move(json)), path);

    return true;
}

void PrefabHandler::unload(Handle& asset) {
    asset = Handle{};
}

void PrefabHandler::instantiate(const Handle& prefab, Scene& scene) {

    if (!prefab.get()) {
        return;
    }

    PrefabSerializer::instantiate(scene, *prefab.get(), _assetManager);
}

} // namespace dzemikk