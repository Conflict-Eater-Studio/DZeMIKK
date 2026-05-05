#include "ecs/serialize/prefabSerializer.h"

#include "boost/uuid/random_generator.hpp"
#include "ecs/components/monoBehaviour.h"
#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/gameobjectSerializer.h"

#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace dzemikk {
nlohmann::json PrefabSerializer::serialize(const GameObject& rootGameObject) {
    nlohmann::json json;
    json["root"] = GameObjectSerializer::serialize(rootGameObject);
    return json;
}

GameObject* PrefabSerializer::instantiate(Scene& scene, const nlohmann::json& json, AssetManager* assetManager,
                                          GameObject* parent) {
    if (!json.contains("root") || !json["root"].is_object()) {
        throw std::runtime_error("Prefab JSON must contain object field 'root'");
    }

    // Create the GameObject hierarchy from JSON
    // Here UUIDs will be wrong -> taken from json
    auto* obj = GameObjectSerializer::instantiateIntoScene(scene, json["root"], parent, assetManager);

    // Resolve script references based on serialized UUIDs before regenerating new ones to ensure
    // correct linking
    std::unordered_map<boost::uuids::uuid, GameObject*> gameObjectsBySerializedId;
    std::unordered_map<boost::uuids::uuid, Component*> componentsBySerializedId;
    std::vector<MonoBehaviour*> monoBehaviours;

    std::vector<GameObject*> toVisit{obj};
    while (!toVisit.empty()) {
        GameObject* current = toVisit.back();
        toVisit.pop_back();

        gameObjectsBySerializedId[current->getId()] = current;

        for (const auto& component : current->getAllComponents()) {
            componentsBySerializedId[component->getId()] = component.get();

            auto* mono = dynamic_cast<MonoBehaviour*>(component.get());
            if (mono != nullptr) {
                monoBehaviours.push_back(mono);
            }
        }

        const auto& children = current->getChildren();
        toVisit.insert(toVisit.end(), children.begin(), children.end());
    }

    // Link scripts via serialized UUIDs while old IDs are still present.
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

    // Regenerate UUIDs for each GameObject and Component to ensure uniquenss
    boost::uuids::random_generator uuid;
    auto objs = std::vector<GameObject*>{obj};
    while (!objs.empty()) {
        auto current = objs;
        objs.clear();

        std::ranges::for_each(current, [&uuid, &objs](GameObject* obj) {
            obj->setId(uuid());
            for (const auto& comp : obj->getAllComponents()) {
                comp->setId(uuid());
            }

            objs.insert(objs.end(), obj->getChildren().begin(), obj->getChildren().end());
        });
    }

    return obj;
}
} // namespace dzemikk
