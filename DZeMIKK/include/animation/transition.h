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

}

#endif
