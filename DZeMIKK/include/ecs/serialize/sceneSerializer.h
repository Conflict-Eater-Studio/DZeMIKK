#pragma once
#ifndef DZEMIKK_SCENESERIALIZER_H
#define DZEMIKK_SCENESERIALIZER_H

#include <nlohmann/json.hpp>

namespace dzemikk {
class AssetManager;
class Scene;

class SceneSerializer {
    public:
        static nlohmann::json serialize(const Scene& scene);
        static void deserializeInto(Scene& scene, const nlohmann::json& json, AssetManager* assetManager);

    };
}

#endif
