#pragma once
#ifndef DZEMIKK_TRANSITION_H
#define DZEMIKK_TRANSITION_H
#include "nlohmann/json.hpp"

#include <functional>
#include <string>
namespace dzemikk {
    struct Transition {
        using Condition = std::function<bool()>;

        float duration;

        std::string targetState;
        Condition condition;
    };

    // inline void to_json(nlohmann::json& json, const Transition& t) {
    //     json["to"] = t.targetState;
    //     json["duration"] = t.duration;
    //     json["condition"] = t.condition;
    // }
    // inline void from_json(const nlohmann::json& json, Transition& t) {
    //     t.targetState = json["to"];
    //     t.condition = json["condition"];
    // }

}

#endif
