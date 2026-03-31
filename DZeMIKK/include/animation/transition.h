#pragma once

#ifndef DZEMIKK_TRANSITION_H
#define DZEMIKK_TRANSITION_H
#include <functional>
#include <string>
struct Transition {
    std::string targetState;
    std::function<bool()> condition;
};
#endif
