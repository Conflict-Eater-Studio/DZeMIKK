#pragma once
#ifndef DZEMIKK_TRANSITION_H
#define DZEMIKK_TRANSITION_H
#include "nlohmann/json.hpp"
#include <string>

namespace dzemikk {
    enum Operator {
        Greater, Less, Equal, NotEqual
    };
    struct Condition {
        std::string parameter;
        Operator op;
        float value;
    };
    struct Transition {
        float duration;
        std::string targetState;
        Condition condition;
    };
    inline void to_json(nlohmann::json& json, const Condition& c) {
        json["parameter"] = c.parameter;
        json["duration"] = c.op;
        json["condition"] = c.value;
    }
    inline void from_json(const nlohmann::json& json, Condition& c) {
        c.parameter = json["parameter"];
        c.op = json["op"];
        c.value = json["value"];
    }
    inline void to_json(nlohmann::json& json, const Transition& t) {
        json["to"] = t.targetState;
        json["duration"] = t.duration;
        json["condition"] = t.condition;
    }
    inline void from_json(const nlohmann::json& json, Transition& t) {
        t.targetState = json["to"];
        t.duration = json["duration"];
        t.condition = json["condition"];
    }
}

#endif
