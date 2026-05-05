#pragma once
#ifndef DZEMIKK_ANIMATORSERIALIZER_H
#define DZEMIKK_ANIMATORSERIALIZER_H

#include "animation/animationclip.h"
#include "animation/animationstate.h"
#include "animation/animationstatemachine.h"
#include "ecs/components/animator.h"
#include "ecs/components/skinnedMeshRenderer.h"
#include "ecs/components/meshRenderer.h"
#include "ecs/gameobject.h"
#include "ecs/serialize/componentSerializerRegistry.h"

#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

namespace dzemikk {
    inline void to_json(nlohmann::json& json, const Condition& c) {
        json["parameter"] = c.parameter;
        json["op"] = c.op;
        json["value"] = c.value;
    }
    inline void from_json(const nlohmann::json& json, Condition& c) {
        c.parameter = json.at("parameter").get<std::string>();
        c.op = static_cast<Operator>(json.at("op").get<int>());
        c.value = json.at("value");
    }
    inline void to_json(nlohmann::json& json, const Transition& t) {
        json["to"] = t.targetState;
        json["duration"] = t.duration;
        json["condition"] = t.condition;
    }
    inline void to_json(nlohmann::json& json, const Animator& animator) {
        json["type"] = animator.typeName();
        json["id"] = boost::uuids::to_string(animator.getId());

        if (auto sm = animator.getStateMachine()) {
            nlohmann::json statesArray = nlohmann::json::array();
            for (const auto& [name, state] : sm->getStates()) {
                nlohmann::json stateJson;
                stateJson["name"] = name;
                stateJson["clip"] = state->getClip()->getName();
                nlohmann::json transArray = nlohmann::json::array();
                for (const auto& trans : state->getTransitions()) {
                    nlohmann::json tJson;
                    to_json(tJson, trans);
                    transArray.push_back(tJson);
                }
                stateJson["transitions"] = transArray;
                statesArray.push_back(stateJson);
            }
            json["stateMachine"] = statesArray;
        }

        if (animator.getCurrentState()) {
            json["currentStateName"] = animator.getCurrentState()->getName();
        }
        json["currentTime"] = animator.getCurrentTime();

        auto& p = json["parameters"] = nlohmann::json::object();
        for (const auto& [name, val] : animator.getFloatParams()) p[name] = val;
        for (const auto& [name, val] : animator.getIntParams())   p[name] = val;
        for (const auto& [name, val] : animator.getBoolParams())  p[name] = val;
    }

    inline void from_json(const nlohmann::json& json, Animator& animator) {
        if (!json.contains("type") || !json["type"].is_string() || json["type"] != animator.typeName()) {
            throw std::runtime_error("Invalid component type for Animator deserialization");
        }

        if (!json.contains("id") || !json.contains("stateMachine") || !json["stateMachine"].is_array() || !json.contains("parameters") || !json["parameters"].is_object() || !json.contains("currentStateName") ) {
            throw std::runtime_error("Missing fields for Animator deserialization");
        }

        animator.setId(boost::uuids::string_generator()(json["id"].get<std::string>()));

        auto sm = std::make_shared<AnimationStateMachine>();

        for (const auto& stateJson : json["stateMachine"]) {
            AnimationState* state = sm->addState(stateJson.at("name").get<std::string>());
            if (stateJson.contains("clip")) {
                auto skinnedMeshRenderer = animator.getOwner()->getComponent<SkinnedMeshRenderer>();
                if (skinnedMeshRenderer != nullptr) {
                    AnimationClip* clip = skinnedMeshRenderer->getModel().get()->getSkeleton()->getClip(stateJson["clip"]);
                    state->setClip(clip);
                }
                auto meshRenderer = animator.getOwner()->getComponent<MeshRenderer>();
                if (meshRenderer != nullptr) {
                    AnimationClip* clip2 = meshRenderer->getModel()->getSkeleton()->getClip(stateJson["clip"]);
                    state->setClip(clip2);
                }
            }
        }

        for (const auto& stateJson : json["stateMachine"]) {
            auto* sourceState = sm->getState(stateJson.at("name").get<std::string>());
            if (stateJson.contains("transitions") && stateJson["transitions"].is_array()) {
                for (const auto& tJson : stateJson["transitions"]) {
                    auto* targetState = sm->getState(tJson.at("to").get<std::string>());
                    if (targetState) {
                        Transition t;
                        t.targetState = targetState->getName();
                        t.duration = tJson.at("duration").get<float>();
                        t.condition = tJson.at("condition").get<Condition>();
                        sourceState->addTransition(t);
                    }
                }
            }
        }
        animator.setStateMachine(sm);

        for (auto& [name, value] : json["parameters"].items()) {
            if (value.is_boolean())         animator.setBool(name, value);
            else if (value.is_number_integer()) animator.setInt(name, value);
            else if (value.is_number_float())   animator.setFloat(name, value);
        }

        animator.play(json["currentStateName"].get<std::string>());
    }

    inline void registerAnimatorSerializer(ComponentSerializerRegistry& registry) {
        registry.registerType(
            "Animator",
            [](const Component& component) {
                const auto* animator = dynamic_cast<const Animator*>(&component);
                nlohmann::json j;
                to_json(j, *animator);
                return j;
            },
            [](const ComponentSerializerRegistry::DeserializationContext& context) {
                auto* animator = context.gameObject.addComponent<Animator>();
                from_json(context.json, *animator);
            });
    }
} // namespace dzemikk

#endif