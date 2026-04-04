#include "ecs/serialize/gameobjectSerializer.h"

#include "ecs/gameobject.h"
#include "ecs/scene.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>


namespace dzemikk {
nlohmann::json GameObjectSerializer::serialize(const GameObject& gameObject) {
    nlohmann::json json;
    json["id"] = boost::uuids::to_string(gameObject.getId());
    json["name"] = gameObject.getName();

    json["components"] = nlohmann::json::array();
    const auto& componentRegistry = ComponentSerializerRegistry::get();
    for (const auto& component : gameObject.getAllComponents()) {
        if (component == nullptr) {
            continue;
        }

        json["components"].push_back(componentRegistry.serialize(*component));
    }

    json["children"] = nlohmann::json::array();
    for (const auto& child : gameObject.getChildren()) {
        if (child == nullptr) {
            continue;
        }
        json["children"].push_back(serialize(*child));
    }

    return json;
}

void GameObjectSerializer::deserializeInto(GameObject& gameObject, const nlohmann::json& json) {
    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("id") || !json["id"].is_string()) {
        throw std::runtime_error("GameObject JSON must contain string field 'id'");
    }

    gameObject.setId(uuidGenerator(json["id"].get<std::string>()));

    if (json.contains("name") && json["name"].is_string()) {
        gameObject.setName(json["name"].get<std::string>());
    }

    if (json.contains("components") && json["components"].is_array()) {
        const auto& componentRegistry = ComponentSerializerRegistry::get();
        for (const auto& componentJson : json["components"]) {
            componentRegistry.deserializeIntoGameObject(gameObject, componentJson);
        }
    }
}

GameObject* GameObjectSerializer::instantiateIntoScene(Scene& scene, const nlohmann::json& json,
                                                       GameObject* parent) {
    GameObject* gameObject = scene.createGameObject();
    deserializeInto(*gameObject, json);

    if (parent != nullptr) {
        parent->addChild(gameObject);
    }

    if (json.contains("children") && json["children"].is_array()) {
        for (const auto& childJson : json["children"]) {
            instantiateIntoScene(scene, childJson, gameObject);
        }
    }

    return gameObject;
}

std::unique_ptr<GameObject> GameObjectSerializer::deserialize(const nlohmann::json& json) {
    static boost::uuids::string_generator uuidGenerator;

    if (!json.contains("id") || !json["id"].is_string()) {
        throw std::runtime_error("GameObject JSON must contain string field 'id'");
    }

    auto gameObject = std::make_unique<GameObject>(uuidGenerator(json["id"].get<std::string>()));
    deserializeInto(*gameObject, json);

    if (json.contains("children") && json["children"].is_array() && !json["children"].empty()) {
        throw std::runtime_error(
            "Detached GameObject deserialize cannot rebuild children without Scene ownership");
    }

    return gameObject;
}
} // namespace dzemikk