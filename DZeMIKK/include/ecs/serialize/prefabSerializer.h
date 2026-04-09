#ifndef DZEMIKK_PREFABSERIALIZER_H
#define DZEMIKK_PREFABSERIALIZER_H

#pragma once

#include <nlohmann/json.hpp>

namespace dzemikk {
class GameObject;
class Scene;

class PrefabSerializer {
  public:
    static nlohmann::json serialize(const GameObject& rootGameObject);
    static GameObject* instantiate(Scene& scene, const nlohmann::json& json,
                                   GameObject* parent = nullptr);
};
} // namespace dzemikk

#endif // DZEMIKK_PREFABSERIALIZER_H
