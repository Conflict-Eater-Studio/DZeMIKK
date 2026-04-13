#ifndef DZEMIKK_GAMEOBJECTSERIALIZER_H
#define DZEMIKK_GAMEOBJECTSERIALIZER_H

#include <memory>
#include <nlohmann/json.hpp>


namespace dzemikk {
class GameObject;
class Scene;
class GameObjectSerializer {
  public:
    static nlohmann::json serialize(const GameObject& gameObject);
    static void deserializeInto(GameObject& gameObject, const nlohmann::json& json);
    static GameObject* instantiateIntoScene(Scene& scene, const nlohmann::json& json,
                                            GameObject* parent = nullptr);
    static std::unique_ptr<GameObject> deserialize(const nlohmann::json& json);
};
} // namespace dzemikk

#endif // DZEMIKK_GAMEOBJECTSERIALIZER_H