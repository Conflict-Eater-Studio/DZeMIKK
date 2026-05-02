#include "ecs/serialize/sceneSerializer.h"

#include "ecs/components/monoBehaviour.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/gameobjectSerializer.h"
#include "assetManager/assetmanager.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>
#include <unordered_map>
#include <vector>


namespace dzemikk {
nlohmann::json SceneSerializer::serialize(const Scene& scene) {
    nlohmann::json json;
    json["id"] = boost::uuids::to_string(scene.getId());
    json["roots"] = nlohmann::json::array();

    for (const auto& object : scene.getObjects()) {
        if (object == nullptr || object->getParent() != nullptr) {
            continue;
        }

        json["roots"].push_back(GameObjectSerializer::serialize(*object));
    }

    return json;
}

void SceneSerializer::deserializeInto(Scene& scene, const nlohmann::json& json) {
    static boost::uuids::string_generator uuidGenerator;

    if (json.contains("id") && json["id"].is_string()) {
        scene.setId(uuidGenerator(json["id"].get<std::string>()));
    }

    if (!json.contains("roots") || !json["roots"].is_array()) {
        throw std::runtime_error("Scene JSON must contain array field 'roots'");
    }

    for (const auto& rootJson : json["roots"]) {
        GameObjectSerializer::instantiateIntoScene(scene, rootJson, nullptr);
    }

    // Resolve UUID-backed script references after all objects/components are instantiated.
    std::unordered_map<boost::uuids::uuid, GameObject*> gameObjectsBySerializedId;
    std::unordered_map<boost::uuids::uuid, Component*> componentsBySerializedId;
    std::vector<MonoBehaviour*> monoBehaviours;

    for (const auto& object : scene.getObjects()) {
        if (object == nullptr) {
            continue;
        }

        GameObject* gameObject = object.get();
        gameObjectsBySerializedId[gameObject->getId()] = gameObject;

        for (const auto& component : gameObject->getAllComponents()) {
            if (component == nullptr) {
                continue;
            }

            Component* rawComponent = component.get();
            componentsBySerializedId[rawComponent->getId()] = rawComponent;

            auto* mono = dynamic_cast<MonoBehaviour*>(rawComponent);
            if (mono != nullptr) {
                monoBehaviours.push_back(mono);
            }
        }
    }

    for (MonoBehaviour* mono : monoBehaviours) {
        mono->resolveSerializedReferences(
            [&gameObjectsBySerializedId](const boost::uuids::uuid& uuidValue) -> GameObject* {
                const auto iter = gameObjectsBySerializedId.find(uuidValue);
                return iter == gameObjectsBySerializedId.end() ? nullptr : iter->second;
            },
            [&componentsBySerializedId](const boost::uuids::uuid& uuidValue) -> Component* {
                const auto iter = componentsBySerializedId.find(uuidValue);
                return iter == componentsBySerializedId.end() ? nullptr : iter->second;
            });
    }
}
void SceneSerializer::deserializeInto(Scene& scene, const nlohmann::json& json, AssetManager& assetManager) {}
} // namespace dzemikk
