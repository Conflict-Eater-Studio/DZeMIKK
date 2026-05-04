#ifndef DZEMIKK_ANIMATORSERIALIZER_H
#define DZEMIKK_ANIMATORSERIALIZER_H
#include "../../components/animator.h"
#include "../componentSerializerRegistry.h"
#include "ecs/gameobject.h"
#include "nlohmann/json.hpp"

#include "ecs/serialize/componentSerializerRegistry.h"
#include "assetManager/assetmanager.h" // Assuming this is where your AssetManager lives

namespace dzemikk {

    inline void to_json(nlohmann::json& json, const Animator& animator) {
        json["type"] = animator.typeName();
        json["id"] = boost::uuids::to_string(animator.getId());

        if (auto sm = animator.getStateMachine()) {
            json["stateMachinePath"] = sm->getPath();
        } else {
            json["stateMachinePath"] = nullptr;
        }

        if (animator.getCurrentState()) {
            json["currentState"] = animator.getCurrentState()->getName();
        }

        // Serialize runtime parameters
        json["parameters"] = nlohmann::json::object();
        // You'll need a way to iterate _parameters if they should persist,
        // but often Animator parameters are reset on load.
        // If you want to save them, add a getter for the map.
    }

    inline void from_json(const nlohmann::json& json, Animator& animator) {
        if (json.contains("id")) {
            animator.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));
        }

        if (json.contains("stateMachinePath") && !json["stateMachinePath"].is_null()) {
            std::string path = json["stateMachinePath"];
            auto sm = AssetManager::get().load<AnimationStateMachine>(path);
            animator.setStateMachine(sm);
        }

        if (json.contains("currentState") && !json["currentState"].is_null()) {
            animator.play(json["currentState"].get<std::string>());
        }
    }

    inline void registerAnimatorSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "Animator",
            [](const Component& component) {
                const auto* animator = dynamic_cast<const Animator*>(&component);
                if (animator == nullptr) {
                    throw std::runtime_error("Component type mismatch for Animator serialization");
                }
                nlohmann::json j;
                to_json(j, *animator);
                return j;
            },
            [](const ComponentSerializerRegistry::DeserializationContext& context) {
                auto* animator = gameObject.addComponent<Animator>();
                from_json(componentJson, *animator, context.assetManager);
            });
    }
}
#endif