#pragma once
#ifndef DZEMIKK_GAMEOBJECTSERIALIZER_H
#define DZEMIKK_GAMEOBJECTSERIALIZER_H

#include <memory>
#include <nlohmann/json.hpp>

namespace dzemikk {
class GameObject;
class Scene;
class AssetManager;
class GameObjectSerializer {
  public:
    static nlohmann::json serialize(const GameObject& gameObject);
    static void deserializeInto(GameObject& gameObject, const nlohmann::json& json, AssetManager* assetManager);
    static GameObject* instantiateIntoScene(Scene& scene, const nlohmann::json& json,
                                                       GameObject* parent, AssetManager* assetManager);
    static std::unique_ptr<GameObject> deserialize(const nlohmann::json& json, AssetManager* assetManager);
};
}

#endif