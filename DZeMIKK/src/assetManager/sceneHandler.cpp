#include "assetManager/sceneHandler.h"

#include "assetManager/assetmanager.h"
#include "ecs/serialize/sceneSerializer.h"
#include "ecs/gameobject.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace dzemikk {

SceneHandler::SceneHandler(AssetManager* assetManager) : _assetManager(assetManager) {}

SceneHandler::Result SceneHandler::load(const std::string& path,
                                        LoadExecutionMode loadExecutionMode) {

    std::ifstream file(path);

    if (!file.is_open()) {
        return {};
    }

    nlohmann::json sceneJson;
    file >> sceneJson;

    auto scene = std::make_shared<Scene>();

    SceneSerializer::deserializeInto(*scene, sceneJson, _assetManager);

    return Result(scene);
}

bool SceneHandler::reload(Handle& asset, const std::string& path) {

    auto* scene = asset.get();
    if (!scene) {
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json sceneJson;
    file >> sceneJson;

    scene->clearAllObjects();

    SceneSerializer::deserializeInto(*scene, sceneJson, _assetManager);

    return true;
}

void SceneHandler::unload(Handle& asset) {
    asset = Handle{};
}

} // namespace dzemikk