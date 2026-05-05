#pragma once
#ifndef DZEMIKK_TRANSITION_H
#define DZEMIKK_TRANSITION_H
#include <functional>
#include <string>

struct Transition {
    using Condition = std::function<bool()>;

    float duration;

    std::string targetState;
    Condition condition;
};
#endif
